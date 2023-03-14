
#include <iostream>
#include <string>
#include <Util/KeyValue.h>
#include "CppParser.h"

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

	if (ProjectType != ENGINE_DLL)
	{
		FKeyValue kv(ToWString(targetPath));
		if (!kv.IsOpen())
		{
			std::cerr << "error: failed to open project file!";
			return 1;
		}

		KVCategory* projects = nullptr;
		if (ProjectType == GAME_PROJECT)
			projects = kv.GetCategory("games");
		else if (ProjectType == DLC_PROJECT)
			projects = kv.GetCategory("dlc");

		if (!projects)
		{
			std::cerr << "error: main.cpp-100";
			return 1;
		}

		KVCategory* target = projects->GetCategory(projectName);
		if (!target)
		{
			std::cerr << "error: failed to find target in project file!";
			return 1;
		}

		KVCategory* dependencies = target->GetCategory("dependencies");
		for (auto dep : dependencies->GetCategories())
		{
			KVValue* srcPath = dep->GetValue("src");
			CParser::LoadModuleData(srcPath->Value);
		}

		targetPath.Erase(targetPath.begin() + targetPath.FindLastOf("/\\"), targetPath.end());
		targetPath += "\\";
		targetPath += projectName + "\\";
	}
	else
		projectName = "Engine";
	
	GeneratedOutput = targetPath + "Intermediate\\generated";

	std::cout << "Running HeaderTool for project: " << projectName.c_str() << std::endl;

	CreateDirectory(GeneratedOutput.c_str(), NULL);

	try
	{
		FString path = targetPath + "src\\";
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
