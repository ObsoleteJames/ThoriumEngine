
#define UTIL_STD_STRING
#include <iostream>
#include <string>
#include <Util/KeyValue.h>
#include "CppParser.h"

#if _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <fstream>
#include <filesystem>
#include <sys/types.h>
#include <sys/stat.h>

#define ENGINE_VERSION "1.0"

#if _WIN32
void ParseCmdLine(LPSTR cmd, TArray<FString>& out)
{
	bool bInQoute = false;

	LPSTR ptr = cmd;
	FString curArg;

	while (true)
	{
		if (*ptr == '\0')
		{
			if (!curArg.IsEmpty())
				out.Add(curArg);
			break;
		}

		if (*ptr == '"')
		{
			bInQoute ^= 1;
		}
		else if (!bInQoute && (*ptr == ' ' || *ptr == '\t'))
		{
			out.Add(curArg);
			curArg.Clear();
		}
		else
			curArg += *ptr;

		ptr++;
	}
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int showCmd)
#else
int main(int argc, char** argv)
#endif
{
#if _WIN32
	TArray<FString> _args;
	_args.Add("Hello!!"); // this is required since 'cmdLine' doesn't include the exe path
	ParseCmdLine(cmdLine, _args);

	char* argv[128];
	int argc = (int)_args.Size();
	for (int i = 0; i < argc; i++)
		argv[i] = (char*)_args[i].c_str();
#endif

	// Parse arguments
	// FString cmd = GetCommandLine();
	// TArray<FString> Args;
	// Args.Resize(1);
	
	// {
	// 	bool bInQuotes = false;
	// 	bool bPrevWasSpace = false;
	// 	for (SizeType i = 0; i < cmd.Size(); i++)
	// 	{
	// 		if (cmd[i] == '"')
	// 		{
	// 			bInQuotes ^= 1;
	// 			continue;
	// 		}

	// 		if (cmd[i] == ' ' && !bInQuotes && !bPrevWasSpace)
	// 		{
	// 			Args.Add(FString());
	// 			bPrevWasSpace = true;
	// 			continue;
	// 		}

	// 		bPrevWasSpace = false;
	// 		(*Args.last()) += cmd[i];
	// 	}
	// }

	// Args.Erase(Args.begin());

	if (argc > 1)
	{
		targetPath = argv[1];
		if (targetPath[0] == ' ')
			targetPath.Erase(targetPath.begin());
		if (targetPath[targetPath.Size() - 1] == '\\' || targetPath[targetPath.Size() - 1] == '/')
			targetPath.Erase(targetPath.last());
	}
	else
	{
		std::cout << "Thorium Engine - Header Tool 1.0\n";
		return 0;
	}

	std::cout << "Path: " << targetPath.c_str() << std::endl;

	bool bIgnoreTime = false;
	for (SizeType i = 2; i < argc; i++)
	{
		FString arg = argv[i];
		/*if (arg == "-config" && i + 1 < argc)
		{
			config = argv[i + 1];
			continue;
		}
		if (arg == "-platform" && i + 1 < argc)
		{
			platform = argv[i + 1];
			continue;
		}*/
		if (arg == "-pt" && i + 1 < argc)
		{
			ProjectType = (EProjectType)std::stoi(argv[i + 1]);
			continue;
		}
		if (arg == "-NoTimestamp")
		{
			bIgnoreTime = true;
			continue;
		}
		if (arg == "-target" && i + 1 < argc)
		{
			projectName = argv[i + 1];
			continue;
		}
	}

	FString enginePath;

	if (ProjectType == GAME_PROJECT)
	{
		FKeyValue projCfg(targetPath + "/../../config/project.cfg");
		if (!projCfg.IsOpen())
		{
			std::cerr << "error: failed to open project file!";
			return 1;
		}

#if _WIN32
		FString keyPath = "SOFTWARE\\ThoriumEngine\\" + *projCfg.GetValue("engine_version");

		HKEY hKey;
		LONG lRes = RegOpenKeyEx(HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_READ, &hKey);
		if (lRes == ERROR_FILE_NOT_FOUND)
			return 1;

		CHAR strBuff[MAX_PATH];
		DWORD buffSize = sizeof(strBuff);
		lRes = RegQueryValueEx(hKey, "path", 0, NULL, (LPBYTE)strBuff, &buffSize);
		if (lRes != ERROR_SUCCESS)
			return 1;

		enginePath = strBuff;
#else
		{
			std::ifstream stream(std::string(getenv("HOME")) + "/.thoriumengine/" + projCfg.GetValue("engine_version")->Value.c_str() + "/path.txt", std::ios_base::in);
			if (stream.is_open())
			{
				std::string str;
				std::getline(stream, str);

				enginePath = str;
			}
		}
#endif

		CParser::LoadModuleData(enginePath + "/build/include/engine");

		projectName = *projCfg.GetValue("game");

		/*auto* addons = projCfg.GetArray("addons");
		if (addons)
		{
			for (auto a : *addons)
			{
				bool bCore;


			}
		}*/

		//KVCategory* projects = nullptr;
		//if (ProjectType == GAME_PROJECT)
		//	projects = kv.GetCategory("games");
		//else if (ProjectType == DLC_PROJECT)
		//	projects = kv.GetCategory("dlc");

		//if (!projects)
		//{
		//	std::cerr << "error: thproj file invalid";
		//	return 1;
		//}

		//KVCategory* target = projects->GetCategory(projectName);
		//if (!target)
		//{
		//	std::cerr << "error: failed to find target in project file!";
		//	return 1;
		//}

		//KVCategory* dependencies = target->GetCategory("dependencies");
		//for (auto dep : dependencies->GetCategories())
		//{
		//	KVValue* srcPath = dep->GetValue("src");
		//	CParser::LoadModuleData(srcPath->Value);
		//}

		//targetPath.Erase(targetPath.begin() + targetPath.FindLastOf("/\\"), targetPath.end());
		//targetPath += "\\";
		//targetPath += projectName + "\\";
	}
	else if (ProjectType == LIBRARY_PROJECT)
	{
#if _WIN32
		FString keyPath = FString("SOFTWARE\\ThoriumEngine\\") + ENGINE_VERSION;

		HKEY hKey;
		LONG lRes = RegOpenKeyEx(HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_READ, &hKey);
		if (lRes == ERROR_FILE_NOT_FOUND)
			return 1;

		CHAR strBuff[MAX_PATH];
		DWORD buffSize = sizeof(strBuff);
		lRes = RegQueryValueEx(hKey, "path", 0, NULL, (LPBYTE)strBuff, &buffSize);
		if (lRes != ERROR_SUCCESS)
			return 1;

		enginePath = strBuff;
#else
		{
			std::ifstream stream(std::string(getenv("HOME")) + "/.thoriumengine/" + ENGINE_VERSION + "/path.txt", std::ios_base::in);
			if (stream.is_open())
			{
				std::string str;
				std::getline(stream, str);

				enginePath = str;
			}
		}
#endif

		FKeyValue kv(targetPath + "/addon.cfg");
		if (!kv.IsOpen())
		{
			std::cerr << "error: failed to open addon config!";
			return 1;
		}

		projectName = *kv.GetValue("identity");

		CParser::LoadModuleData(enginePath + "/build/include/engine");
	}
	else
		projectName = "Engine";
	
	GeneratedOutput = targetPath + "\\Intermediate\\generated";

	std::cout << "Running HeaderTool for project: " << projectName.c_str() << std::endl;

	//CreateDirectory(GeneratedOutput.c_str(), NULL);
	std::filesystem::create_directories(GeneratedOutput.c_str());

	try
	{
		FString path = targetPath + "\\src\\";
		for (auto entry : std::filesystem::recursive_directory_iterator(path.c_str()))
		{
			if (!entry.is_regular_file())
				continue;

			if (entry.path().extension() != ".h")
				continue;

			FHeaderData header;
			header.FileName = entry.path().stem().generic_string();
			header.FilePath = entry.path().generic_string();

			CParser::ParseHeader(header);

			if (!header.bEmpty)
				Headers.Add(header);
		}
	}
	catch (std::exception e) { std::cerr << "error: " << e.what(); }

	for (auto& h : Headers)
	{
		if (!bIgnoreTime && CParser::HeaderUpToDate(h))
			continue;

		CParser::WriteGeneratedHeader(h);
		CParser::WriteGeneratedCpp(h);
	}

	CParser::WriteModuleCpp();
	CParser::WriteModuleData();

	CParser::WriteTimestamp();

#if _WIN32
	if (IsDebuggerPresent())
	{
		std::cout << "Press enter to continue...";
		std::cin.get();
	}
#endif

	return 0;
}
