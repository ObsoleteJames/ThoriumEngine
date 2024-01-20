
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include "Build.h"
#include <Util/KeyValue.h>
#include <chrono>
#include <thread>

#if _WIN32
#include <Windows.h>
#else
#include "unistd.h"
#include <spawn.h>
#include <sys/wait.h>

extern char** environ;
#endif

const char* PlatformStrings[] = {
	"win64",
	"linux",
	"macos"
};

const char* ConfigStrings[] = {
	"Debug",
	"Development",
	"Release"
};

int CompileMSVC(FKeyValue& buildCfg, EPlatform platform, EConfig config, const FString& msvcPath);
int CompileClang(const FString& path, EPlatform platform, EConfig config);
int CompilerGcc(const FString& path, EPlatform platform, EConfig config);

class FStringExpression
{
public:
	void SetExpressionValue(const FString& key, const FString& value);
	FString GetExpression(const FString& key);

	FString ParseString(const FString& in);

private:
	TArray<TPair<FString, FString>> expressions;
};

void FStringExpression::SetExpressionValue(const FString& key, const FString& value)
{
	expressions.Add({ key.ToLowerCase(), value });
}

FString FStringExpression::GetExpression(const FString& _k)
{
	FString key = _k.ToLowerCase();
	for (auto& k : expressions)
		if (k.Key == key)
			return k.Value;
	return FString();
}

FString FStringExpression::ParseString(const FString& in)
{
	FString curExpression;
	int mode = 0;

	FString out;

	for (int i = 0; i < in.Size(); i++)
	{
		if (mode == 0)
		{
			if (in[i] == '$' && in[i + 1] == '{')
			{
				curExpression.Clear();
				mode = 1;
				i++;
				continue;
			}

			out += in[i];
		}
		else if (mode == 1)
		{
			if (in[i] == '}')
			{
				out += GetExpression(curExpression);
				mode = 0;
				continue;
			}

			curExpression += in[i];
		}
	}

	return out;
}

bool IsFileExcluded(FKeyValue& buildCfg, const FString& file)
{
	TArray<FString>* exclude = buildCfg.GetArray("Exclude");
	if (!exclude)
		return false;

	for (auto& ex : *exclude)
	{
		if (*ex.last() == '*')
		{
			FString cmpr = ex;
			cmpr.Erase(cmpr.last());
			if (file.Find(cmpr) == 0)
				return true;
			continue;
		}

		if (ex == file)
			return true;
	}
	return false;
}

void GetCppFilesInDir(FKeyValue& buildCfg, const FString& source, TArray<FString>& out)
{
	if (!std::filesystem::exists(source.c_str()))
		return;

	for (std::filesystem::directory_entry entry : std::filesystem::recursive_directory_iterator(source.c_str()))
	{
		if (entry.is_directory())
		{
			//GetCppFilesInDir(buildCfg, entry.path().generic_string().c_str(), out);
			continue;
		}

		if (entry.path().extension() != ".cpp" && entry.path().extension() != ".c")
			continue;

		FString path = entry.path().generic_string().c_str();
		//if (auto i = path.Find("src"); i != -1)
		//	path.Erase(path.begin(), path.begin() + i + 4);

		path.Erase(path.begin(), path.begin() + source.Size() + 1);

		if (IsFileExcluded(buildCfg, path))
			continue;

		out.Add(entry.path().generic_string().c_str());
	}
}

FString GetEnginePath(const FString& version)
{
#if _WIN32
	FString keyPath = "SOFTWARE\\ThoriumEngine\\" + version;

	HKEY hKey;
	LONG lRes = RegOpenKeyEx(HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_READ, &hKey);
	if (lRes == ERROR_FILE_NOT_FOUND)
		return FString();

	CHAR strBuff[MAX_PATH];
	DWORD buffSize = sizeof(strBuff);
	lRes = RegQueryValueEx(hKey, "path", 0, NULL, (LPBYTE)strBuff, &buffSize);
	if (lRes != ERROR_SUCCESS)
		return FString();

	return strBuff;
#else
	std::ifstream stream(std::string(getenv("HOME")) + "/.thoriumengine/" + version.c_str() + "/path.txt", std::ios_base::in);
	if (!stream.is_open())
	{
		std::cerr << "error: failed to obtain engine path!\n";
		return FString();
	}

	std::string str;
	std::getline(stream, str);

	return str.c_str();
#endif
}

int CompileSource(const FCompileConfig& config)
{
	std::cout << "Generating build file for '" << config.path.c_str() << "'\n";

	FString compilerPath;
	FString enginePath;

	FString targetPath = config.path;
	if (auto i = targetPath.FindLastOf("\\/"); i != -1)
		targetPath.Erase(targetPath.begin() + i, targetPath.end());
	targetPath.ReplaceAll('\\', '/');

	FStringExpression strExp;
	strExp.SetExpressionValue("PLATFORM", PlatformStrings[config.platform]);
	strExp.SetExpressionValue("CONFIG", ConfigStrings[config.config]);
	strExp.SetExpressionValue("PATH", targetPath);

	enginePath = GetEnginePath(config.engineVersion);
	if (enginePath.IsEmpty())
		return 1;
	enginePath.ReplaceAll('\\', '/');

	strExp.SetExpressionValue("ENGINE_PATH", enginePath);
	strExp.SetExpressionValue("ENGINE_LIB", enginePath + "/build/" + PlatformStrings[config.platform] + "/Engine-" + ConfigStrings[config.config] + "/Engine.lib");
	strExp.SetExpressionValue("UTIL_LIB", enginePath + "/build/" + PlatformStrings[config.platform] + "/Util-" + ConfigStrings[config.config] + "/Util.lib");

	FKeyValue buildCfg;
	buildCfg.DefineMacro("PLATFORM_WINDOWS", config.platform == PLATFORM_WIN64);
	buildCfg.DefineMacro("PLATFORM_LINUX", config.platform == PLATFORM_LINUX);
	buildCfg.DefineMacro("PLATFORM_MAC", config.platform == PLATFORM_MAC);
	buildCfg.DefineMacro("_DEBUG", config.config == CONFIG_DEBUG);
	buildCfg.DefineMacro("_DEVELOPMENT", config.config == CONFIG_DEVELOPMENT);
	buildCfg.DefineMacro("_RELEASE", config.config == CONFIG_RELEASE);

	buildCfg.Open(config.path);
	if (!buildCfg.IsOpen())
	{
		std::cerr << "error: failed to open build config!\n";
		std::cerr << buildCfg.GetLastError().c_str();

		return 1;
	}

#if _WIN32
	SetCurrentDirectoryA(targetPath.c_str());
#else
	chdir(targetPath.c_str());
#endif

	FString targetBuild = *buildCfg.GetValue("Target");
	strExp.SetExpressionValue("TARGET", targetBuild);

	FString type = *buildCfg.GetValue("Type");
	FString version = *buildCfg.GetValue("Version");
	bool bIsEngine = type == "engine";
	bool bIsGame = type == "game";
	bool bIsLibrary = type == "library" || type == "game";
	bool bIsStaticLib = type == "static_library";
	bool bIsExe = type == "executable";

	FString cmakeProj = targetBuild.ToUpperCase();
	cmakeProj.ReplaceAll('.', '_');
	//FString cmakeLib = cmakeProj.ToLowerCase();
	FString cmakeLib = targetBuild;

	bool bRunHeaderTool = buildCfg.GetValue("ExecuteHeaderTool")->AsBool();

	if (!bIsEngine && !bIsGame && !bIsLibrary && !bIsStaticLib && !bIsExe)
	{
		std::cerr << "error: invalid build type '" << type.c_str() << "'\n";
		return 1;
	}

	// Run HeaderTool
	if (bRunHeaderTool)
	{
#if _WIN32
		PROCESS_INFORMATION ht{};
		STARTUPINFO si{};
		si.cb = sizeof(si);
		FString htCmd = enginePath + "/bin/win64/HeaderTool.exe \"" + targetPath + "\" -pt " + (bIsEngine ? "0" : (bIsGame ? "1" : "3"));
		if (!CreateProcessA(NULL, (char*)htCmd.c_str(), nullptr, nullptr, false, 0, nullptr, nullptr, &si, &ht))
		{
			std::cerr << "error: failed to run HeaderTool!\n";
			return 1;
		}
#else
		FString p = enginePath + "/bin/linux/HeaderTool";

		pid_t pid;
		
		const char* pt = (bIsEngine ? "0" : (bIsGame ? "1" : "3"));
		TArray<const char*> argv = {
			targetPath.c_str(),
			"-pt",
			pt
		};

		int status = posix_spawn(&pid, p.c_str(), NULL, NULL, (char**)argv.Data(), environ);

		if (status != 0)
		{
			std::cerr << "error: failed to run HeaderTool! if this is the first time compiling ignore this.\n";
			//return 1;
		}
		else
			waitpid(pid, &status, 0);
#endif
	}

	using namespace std::chrono_literals;
	std::this_thread::sleep_for(10ms);

	auto* includeOut = buildCfg.GetValue("IncludeOut", false);
	if (includeOut)
	{
		FString io = strExp.ParseString(*includeOut);
		std::filesystem::create_directories(io.c_str());
		CopyHeaders(buildCfg, targetPath + "/src", io);
		CopyHeaders(buildCfg, targetPath + "/intermediate/generated", io);
	}

	std::filesystem::create_directories((targetPath + "/Intermediate").c_str());

	// Generate CMakeLists.txt
	std::ofstream stream;
	stream.exceptions(std::ios::badbit | std::ios::failbit);
	try {
		stream.open((targetPath + "/Intermediate/CMakeLists.txt").c_str(), std::ios_base::trunc);
	} 
	catch(std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

	if (!stream.is_open())
	{
		std::cerr << "error: failed to create file stream! '" << targetPath.c_str() << "/Intermediate/CMakeLists.txt'\n";
		return 1;
	}

	stream << "cmake_minimum_required(VERSION 3.10.0)\n";
	stream << "include_guard(GLOBAL)\n";
	stream << "project(" << cmakeProj.c_str() << ")\n";

	if (config.config == CONFIG_DEBUG)
		stream << "set(CMAKE_BUILD_TYPE Debug)\n";
	if (config.config == CONFIG_DEVELOPMENT)
		stream << "set(CMAKE_BUILD_TYPE RelWithDebInfo)\n";
	if (config.config == CONFIG_RELEASE)
		stream << "set(CMAKE_BUILD_TYPE Release)\n";

	TArray<FString> files;
	GetCppFilesInDir(buildCfg, targetPath + "/src", files);
	GetCppFilesInDir(buildCfg, targetPath + "/intermediate/generated", files);

	stream << "\nset(Files ";

	for (auto& f : files)
		stream << "\n\t" << f.c_str();

	stream << ")\n\n";
	stream << "include_directories(../src ../intermediate/generated ";

	auto* includes = buildCfg.GetArray("Include");
	if (includes)
	{
		for (auto i : *includes)
		{
			i.ReplaceAll('\\', '/');
			stream << "\n\t" << strExp.ParseString(i).c_str();
		}
	}
	stream << ")\n\n";

	auto* dep = buildCfg.GetArray("Dependencies");
	if (dep && dep->Size() > 0)
	{
		for (auto depend : *dep)
		{
			depend = strExp.ParseString(depend);
			FString libTarget;
			if (depend.Find("package:") == 0)
			{
				FString package = depend;
				package.Erase(package.begin(), package.begin() + 8);
				stream << "find_package(" << package.c_str() << " REQUIRED)\n";
				continue;
			}

			if (depend.Find("Build.cfg") != -1)
			{
				FKeyValue depBuild;
				depBuild.DefineMacro("PLATFORM_WINDOWS", config.platform == PLATFORM_WIN64);
				depBuild.DefineMacro("PLATFORM_LINUX", config.platform == PLATFORM_LINUX);
				depBuild.DefineMacro("PLATFORM_MAC", config.platform == PLATFORM_MAC);
				depBuild.DefineMacro("_DEBUG", config.config == CONFIG_DEBUG);
				depBuild.DefineMacro("_DEVELOPMENT", config.config == CONFIG_DEVELOPMENT);
				depBuild.DefineMacro("_RELEASE", config.config == CONFIG_RELEASE);

				depBuild.Open(depend);
				if (!depBuild.IsOpen())
					continue;

				libTarget = *depBuild.GetValue("Target");
				//libOut = *depBuild.GetValue("LibOut");

				FString libPath = depend;
				if (auto i = libPath.FindLastOf("\\/"); i != -1)
					libPath.Erase(libPath.begin() + i, libPath.end());

				FStringExpression libExp;
				libExp.SetExpressionValue("PLATFORM", PlatformStrings[config.platform]);
				libExp.SetExpressionValue("CONFIG", ConfigStrings[config.config]);
				libExp.SetExpressionValue("PATH", libPath);
				libExp.SetExpressionValue("ENGINE_PATH", enginePath);

				FCompileConfig _cfg;
				_cfg.path = depend;
				_cfg.compiler = config.compiler;
				_cfg.platform = config.platform;
				_cfg.config = config.config;
				_cfg.engineVersion = config.engineVersion;

				CompileSource(_cfg);

				FString libFile = libPath + "/Intermediate";
				depend = libFile;
			}

			stream << "add_subdirectory(\"" << depend.c_str() << "\" \"build/" << libTarget.c_str() << "\")\n";
		}
		stream << std::endl;
	}

	const char* platformStr = config.platform == PLATFORM_WIN64 ? "PLATFORM_WINDOWS" 
		: (config.platform == PLATFORM_LINUX ? "PLATFORM_LINUX" : "PLATFORM_MACOS");
	stream << "add_compile_definitions(" << platformStr << " CONFIG_" << FString(ConfigStrings[config.config]).c_str();
	if (config.config != CONFIG_RELEASE)
		stream << " IS_DEV";
	stream << " " << cmakeProj.c_str() << "_DLL)\n";

	if (!bIsExe)
		stream << "add_library(" << cmakeLib.c_str() << (bIsStaticLib ? " STATIC " : " SHARED ") << " ${Files})\n\n";
	else
		stream << "add_executable(" << cmakeLib.c_str() << " WIN32 ${Files})\n\n";

	stream << "set_property(TARGET " << cmakeLib.c_str() << " PROPERTY CXX_STANDARD 17)\n";
	if (config.platform != PLATFORM_WIN64)
		stream << "set_target_properties(" << cmakeLib.c_str() << " PROPERTIES PREFIX \"\" OUTPUT_NAME \"" << targetBuild.c_str() 
			<< "\" IMPORT_PREFIX \"\" IMPORT_SUFFIX \"" << (config.platform == PLATFORM_WIN64 ? ".dll" : ".a") << "\")\n\n";

	dep = buildCfg.GetArray("LinkTargets");
	if (dep)
	{
		for (auto depend : *dep)
		{
			stream << "target_link_libraries(" << cmakeLib.c_str();
			depend = strExp.ParseString(depend);
			bool bIsAddon = false;
			bool bIsBuildFile = false;
			bool bIsCmakeLists = false;
			if (depend.Find("addon:") == 0)
			{
				continue;
				bIsAddon = true;
				depend.Erase(depend.begin(), depend.begin() + 6);
			}
			else if (auto ii = depend.Find("CMakeLists.txt"); ii != -1)
			{
				bIsCmakeLists = true;

				depend.Erase(depend.begin() + ii - 1, depend.end());
			}
			else if (depend.Find("Build.cfg") != -1)
			{
				bIsBuildFile = true;

				FKeyValue depBuild;
				depBuild.DefineMacro("PLATFORM_WINDOWS", config.platform == PLATFORM_WIN64);
				depBuild.DefineMacro("PLATFORM_LINUX", config.platform == PLATFORM_LINUX);
				depBuild.DefineMacro("PLATFORM_MAC", config.platform == PLATFORM_MAC);
				depBuild.DefineMacro("_DEBUG", config.config == CONFIG_DEBUG);
				depBuild.DefineMacro("_DEVELOPMENT", config.config == CONFIG_DEVELOPMENT);
				depBuild.DefineMacro("_RELEASE", config.config == CONFIG_RELEASE);

				depBuild.Open(depend);
				if (!depBuild.IsOpen())
					continue;
				
				FString libTarget = *depBuild.GetValue("Target");
				FString libOut = *depBuild.GetValue("LibOut");

				FString libPath = depend;
				if (auto i = libPath.FindLastOf("\\/"); i != -1)
					libPath.Erase(libPath.begin() + i, libPath.end());

				FStringExpression libExp;
				libExp.SetExpressionValue("PLATFORM", PlatformStrings[config.platform]);
				libExp.SetExpressionValue("CONFIG", ConfigStrings[config.config]);
				libExp.SetExpressionValue("PATH", libPath);
				libExp.SetExpressionValue("ENGINE_PATH", enginePath);

				FString libFile = libExp.ParseString(libOut) + "/" + libTarget + (config.platform == PLATFORM_WIN64 ? ".dll" : ".a");
				depend = libFile;

				// TODO: compile this project
				FCompileConfig _cfg;
				_cfg.path = depend;
				_cfg.compiler = config.compiler;
				_cfg.platform = config.platform;
				_cfg.config = config.config;
				_cfg.engineVersion = config.engineVersion;

				CompileSource(_cfg);
			}

			// TODO: if addon, find the addons lib file
			
			stream << " \"" << depend.c_str() << "\"";
			stream << ")\n";
		}
	}

	FString cmds;

	if (auto* libOut = buildCfg.GetValue("LibOut", false); libOut)
	{
		FString lo = strExp.ParseString(*libOut);
		lo.ReplaceAll('\\', '/');
		if (!bIsStaticLib)
			cmds += "\tCOMMAND ${CMAKE_COMMAND} -E copy \"$<TARGET_FILE:" + cmakeLib + ">\" \"" + lo + "/\"\n";
			// stream << "add_custom_command(TARGET " << cmakeLib.c_str() << " POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy \"$<TARGET_FILE:"
			// 	<< cmakeLib.c_str() << ">\" \"" << lo.c_str() << "\")\n";

		cmds += "\tCOMMAND ${CMAKE_COMMAND} -E copy \"$<TARGET_LINKER_FILE:" + cmakeLib + ">\" \"" + lo + "/\"\n";
		// stream << "add_custom_command(TARGET " << cmakeLib.c_str() << " POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy \"$<TARGET_LINKER_FILE:"
		// 	<< cmakeLib.c_str() << ">\" \"" << lo.c_str() << "\")\n";
	}

	if (includeOut && bRunHeaderTool)
	{
		FString lo = strExp.ParseString(*includeOut);

		cmds += "\tCOMMAND ${CMAKE_COMMAND} -E copy \"" + targetPath + "/Intermediate/module.bin\" \"" + lo + "/module.bin\"\n";
		// stream << "add_custom_command(TARGET " << cmakeLib.c_str() << " POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy \"module.bin\" \""
		// 	<< lo.c_str() << "/module.bin\")\n";
	}

	if (auto* binOut = buildCfg.GetValue("BuildOut", false); binOut)
	{
		FString bo = strExp.ParseString(*binOut);
		bo.ReplaceAll('\\', '/');

		cmds += "\tCOMMAND ${CMAKE_COMMAND} -E copy \"$<TARGET_FILE:" + cmakeLib + ">\" \"" + bo + "/\"\n";
		// stream << "add_custom_command(TARGET " << cmakeLib.c_str() << " POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy \"$<TARGET_FILE:"
		// 	<< cmakeLib.c_str() << ">\" \"" << bo.c_str() << "\")\n";
	}

	if (cmds.Size() > 0)
	{
		stream << "add_custom_command(TARGET " << cmakeLib.c_str() << " POST_BUILD\n" << cmds.c_str() << ")\n";
	}

	stream.close();

	//CompileMSVC(buildCfg, config.platform, config.config, compilerPath);
	return 0;
}

void CopyHeaders(FKeyValue& buildCfg, const FString& source, const FString& oi)
{
	TArray<FString> headers;
	if (!std::filesystem::exists(source.c_str()))
		return;
	for (std::filesystem::directory_entry entry : std::filesystem::recursive_directory_iterator(source.c_str()))
	{
		if (entry.is_directory())
			continue;

		if (entry.path().extension() != ".h" && entry.path().extension() != ".hpp")
			continue;

		FString path = entry.path().generic_string().c_str();
		if (path.Find(source) == 0)
			path.Erase(path.begin(), path.begin() + source.Size() + 1);
		if (IsFileExcluded(buildCfg, path))
			continue;

		headers.Add(entry.path().generic_string().c_str());
	}

	FString out = oi;
	if (*out.last() != '/' || *out.last() != '\\')
		out += '/';

	if (headers.Size() == 0)
	{
		std::cout << "warning: found 0 header files in '" << source.c_str() << "'!\n";
	}

	for (auto& h : headers)
	{
		FString file = h;
		file.Erase(file.begin(), file.begin() + source.Size());

		FString outFile = out + file;

		FString outDir = outFile;
		outDir.Erase(outDir.begin() + outDir.FindLastOf("\\/"), outDir.end());

		try
		{
			std::filesystem::create_directories(outDir.c_str());
			std::filesystem::copy(h.c_str(), outFile.c_str(), std::filesystem::copy_options::overwrite_existing);
		}
		catch (std::exception& e)
		{
			std::cerr << "error: " << e.what() << " in: " << h.c_str() << " - out: " << (out + file).c_str() << std::endl;
		}
	}
}

bool GenerateBuildFromProject(const FString& projectCfg)
{
	std::cout << "Generating Build.cfg from project.\n";

	FKeyValue proj(projectCfg);
	if (!proj.IsOpen())
		return false;

	FString game = *proj.GetValue("game");

	FString buildFile = projectCfg;
	if (auto i = buildFile.FindLastOf("\\/"); i != -1)
		buildFile.Erase(buildFile.begin() + i, buildFile.end());
	if (auto i = buildFile.FindLastOf("\\/"); i != -1)
		buildFile.Erase(buildFile.begin() + i, buildFile.end());

	FString projFolder = buildFile;

	buildFile += "/.project/" + game + "/Build.cfg";

	FKeyValue buildCfg(buildFile);

	game.ReplaceAll('.', '_');
	game.ReplaceAll(' ', '_');
	buildCfg.SetValue("Target", game);
	buildCfg.SetValue("Type", "game");
	buildCfg.SetValue("Version", *proj.GetValue("version"));

	buildCfg.SetValue("ExecuteHeaderTool", "true");
	buildCfg.SetValue("IncludeOut", "${PATH}/include");

	buildCfg.SetValue("LibOut", "${PATH}/bin");
	buildCfg.SetValue("BuildOut", projFolder + "/" + game + "/bin");

	auto* includes = buildCfg.GetArray("Include", true);
	includes->Clear();
	includes->Add("${ENGINE_PATH}/build/include/Engine");
	includes->Add("${ENGINE_PATH}/build/include/Util");

	auto* depend = buildCfg.GetArray("LinkTargets", true);
	depend->Clear();
	depend->Add("${ENGINE_LIB}");
	depend->Add("${UTIL_LIB}");

	auto* addons = proj.GetArray("addons");
	if (addons)
	{
		for (auto a : *addons)
		{
			if (auto i = a.FindLastOf(':'); i != -1)
				a.Erase(a.begin(), a.begin() + i + 1);

			depend->Add("addon:" + a);
		}
	}

	buildCfg.Save();
	return true;
}

int CompileMSVC(FKeyValue& buildCfg, EPlatform platform, EConfig config, const FString& msvcPath)
{
	TArray<FString> filesToCompile;
	GetCppFilesInDir(buildCfg, "src", filesToCompile);

	FString platformStr = PlatformStrings[platform];
	FString configStr = ConfigStrings[config];

	FString clArgs;
	if (config < CONFIG_RELEASE)
		clArgs += "/JMC ";
	clArgs += "/permissive- /ifcOutput \"Intermediate\\" + platformStr + "\\" + configStr + "\\\" /c /GS /W3 ";

	if (config == CONFIG_RELEASE)
		clArgs += "/GL ";
	if (config > CONFIG_DEBUG)
		clArgs += "/Gy ";

	clArgs += "/Zc:wchar_t ";
	return 0;
}

int CompileClang(const FString& path, EPlatform platform, EConfig config)
{
	return 0;
}

int CompilerGcc(const FString& path, EPlatform platform, EConfig config)
{
	return 0;
}

