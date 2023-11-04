
#include <iostream>
#include <string>
#include <Util/KeyValue.h>
#include "CppParser.h"
#include "EngineCore.h"

#include <windows.h>
#include <filesystem>
#include <sys/types.h>
#include <sys/stat.h>

int main()
{
	// Parse arguments
	FString cmd = GetCommandLine();
	TArray<FString> Args;
	Args.Resize(1);
	
	{
		bool bInQuotes = false;
		bool bPrevWasSpace = false;
		for (SizeType i = 0; i < cmd.Size(); i++)
		{
			if (cmd[i] == '"')
			{
				bInQuotes ^= 1;
				continue;
			}

			if (cmd[i] == ' ' && !bInQuotes && !bPrevWasSpace)
			{
				Args.Add(FString());
				bPrevWasSpace = true;
				continue;
			}

			bPrevWasSpace = false;
			(*Args.last()) += cmd[i];
		}
	}

	Args.Erase(Args.begin());

	targetPath = Args[0];
	if (targetPath[0] == ' ')
		targetPath.Erase(targetPath.begin());
	if (targetPath[targetPath.Size() - 1] == '\\' || targetPath[targetPath.Size() - 1] == '/')
		targetPath.Erase(targetPath.last());

	std::cout << "Path: " << targetPath.c_str() << std::endl;

	bool bIgnoreTime = false;
	for (SizeType i = 1; i < Args.Size(); i++)
	{
		FString arg = Args[i];
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
		if (arg == "-pt" && i + 1 < Args.Size())
		{
			ProjectType = (EProjectType)std::stoi(Args[i + 1].c_str());
			continue;
		}
		if (arg == "-NoTimestamp")
		{
			bIgnoreTime = true;
			continue;
		}
		if (arg == "-target" && i + 1 < Args.Size())
		{
			projectName = Args[i + 1];
			continue;
		}
	}

	FString enginePath;

	if (ProjectType == GAME_PROJECT)
	{
		FKeyValue projCfg(ToWString(targetPath + "/../../config/project.cfg"));
		if (!projCfg.IsOpen())
		{
			std::cerr << "error: failed to open project file!";
			return 1;
		}

#if _WIN32
		WString keyPath = ToWString("SOFTWARE\\ThoriumEngine\\" + *projCfg.GetValue("engine_version"));

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
		WString keyPath = ToWString(FString("SOFTWARE\\ThoriumEngine\\") + ENGINE_VERSION);

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
#endif

		FKeyValue kv(ToWString(targetPath + "/addon.cfg"));
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

	CreateDirectory(GeneratedOutput.c_str(), NULL);

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

	if (IsDebuggerPresent())
	{
		std::cout << "Press enter to continue...";
		std::cin.get();
	}

	return 0;
}
