
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include "Build.h"
#include <Util/KeyValue.h>

#if _WIN32
#include <Windows.h>
#endif

const char* PlatformStrings[] = {
	"x64",
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

void GetCppFilesInDir(FKeyValue& buildCfg, const FString& path, TArray<FString>& out)
{
	for (std::filesystem::directory_entry entry : std::filesystem::directory_iterator(path.c_str()))
	{
		if (entry.is_directory())
		{
			GetCppFilesInDir(buildCfg, entry.path().generic_string().c_str(), out);
			continue;
		}

		if (entry.path().extension() != ".cpp" && entry.path().extension() != ".c")
			continue;

		FString path = entry.path().generic_string();
		if (path.Find("src") == 0)
			path.Erase(path.begin(), path.begin() + 4);

		if (IsFileExcluded(buildCfg, path))
			continue;

		out.Add(path);
	}
}

int CompileSource(const FCompileConfig& config)
{
	FString compilerPath;
	FString enginePath;

#if _WIN32
	SetCurrentDirectoryA(config.path.c_str());
#endif

	// Get the compiler path
	if (config.compiler < COMPILER_CLANG)
	{
#if _WIN32
		WString keyPath = ToWString("SOFTWARE\\ThoriumEngine\\" + config.engineVersion);

		HKEY hKey;
		LONG lRes = RegOpenKeyExW(HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_READ, &hKey);
		if (lRes == ERROR_FILE_NOT_FOUND)
			return 1;

		WCHAR strBuff[MAX_PATH];
		DWORD buffSize = sizeof(strBuff);
		lRes = RegQueryValueExW(hKey, L"path", 0, NULL, (LPBYTE)strBuff, &buffSize);
		if (lRes != ERROR_SUCCESS)
			return 1;

		enginePath = ToFString(strBuff);

		HANDLE pRead;
		HANDLE pWrite;
		if (!CreatePipe(&pRead, &pWrite, nullptr, 0))
		{
			std::cerr << "error: failed to locate visual studio installation.\n";
			return 1;
		}

		PROCESS_INFORMATION vswhereInfo{};
		STARTUPINFO info{};
		info.cb = sizeof(info);
		info.hStdOutput = pWrite;

		WString compilerVersion;
		if (config.compiler == COMPILER_MSVC15)
			compilerVersion = L"15";
		else if (config.compiler == COMPILER_MSVC16)
			compilerVersion = L"16";
		else if (config.compiler == COMPILER_MSVC17)
			compilerVersion = L"17";

		if (!CreateProcessW((ToWString(enginePath) + L"\\bin\\vswhere.exe").c_str(), (LPWSTR)(L"-version " + compilerVersion + L" -property installationPath").c_str(), nullptr, nullptr, false, 0, nullptr, nullptr, &info, &vswhereInfo))
		{
			std::cerr << "error: failed to locate visual studio installation.\n";
			return 2;
		}

		WaitForSingleObject(vswhereInfo.hProcess, INFINITE);

		CloseHandle(vswhereInfo.hProcess);
		CloseHandle(vswhereInfo.hThread);

		char outBuff[260];
		if (!ReadFile(pRead, outBuff, 260, 0, nullptr))
		{
			std::cerr << "error: failed to locate visual studio installation.\n";
			return 3;
		}
		CloseHandle(pRead);
		CloseHandle(pWrite);

		compilerPath = FString((const char*)outBuff).Split('\n')[0];
		if (compilerPath.IsEmpty())
		{
			std::cerr << "error: failed to locate visual studio installation.\n";
			return 4;
		}

		std::ifstream vStream((compilerPath + "\\VC\\Auxiliary\\Build\\Microsoft.VCToolsVersion.default.txt").c_str());
		if (!vStream.is_open())
		{
			std::cerr << "error: failed to get msvc version.\n";
			return 1;
		}

		std::string msvcVersion;
		std::getline(vStream, msvcVersion);
		vStream.close();

		if (msvcVersion.empty())
		{
			std::cerr << "error: failed to get msvc version.\n";
			return 2;
		}

		compilerPath += FString("\\VC\\Tools\\MSVC\\") + msvcVersion + "\\bin\\Hostx64\\x64";
#endif
	}

	FKeyValue buildCfg;
	buildCfg.DefineMacro("PLATFORM_WINDOWS", config.platform == PLATFORM_WIN64);
	buildCfg.DefineMacro("PLATFORM_LINUX", config.platform == PLATFORM_LINUX);
	buildCfg.DefineMacro("PLATFORM_MAC", config.platform == PLATFORM_MAC);
	buildCfg.DefineMacro("_DEBUG", config.config == CONFIG_DEBUG);
	buildCfg.DefineMacro("_DEVELOPMENT", config.config == CONFIG_DEVELOPMENT);
	buildCfg.DefineMacro("_RELEASE", config.config == CONFIG_RELEASE);

	buildCfg.Open(config.path + "\\Build.cfg");
	if (!buildCfg.IsOpen())
	{
		std::cerr << "error: failed to locate build config!\n";
		return 1;
	}

	FString targetBuild = *buildCfg.GetValue("Target");
	FString type = *buildCfg.GetValue("Type");
	FString version = *buildCfg.GetValue("Version");
	bool bIsEngine = targetBuild == "Engine";

	// Run HeaderTool
#if _WIN32
	PROCESS_INFORMATION ht{};
	STARTUPINFO si{};
	si.cb = sizeof(si);
	WString htCmd = ToWString(enginePath) + L"\\bin\\HeaderTool.exe \"" + ToWString(config.path) + L"\" -pt" + (type == "Engine" ? L"0" : L"1");
	if (!CreateProcess(NULL, (wchar_t*)htCmd.c_str(), nullptr, nullptr, false, 0, nullptr, nullptr, &si, &ht))
	{
		std::cerr << "error: failed to run HeaderTool!\n";
		return 1;
	}
#endif

	CompileMSVC(buildCfg, config.platform, config.config, compilerPath);
	return 0;
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
}

int CompileClang(const FString& path, EPlatform platform, EConfig config)
{

}

int CompilerGcc(const FString& path, EPlatform platform, EConfig config)
{
	
}

