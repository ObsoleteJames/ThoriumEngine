
#include <Util/KeyValue.h>
#include "Misc/FileHelper.h"
#include "Engine.h"
#include "ToolsCore.h"
#include "Misc/CommandLine.h"
#include "Windows/ProjectManagerWindow.h"
#include "Windows/EditorWindow.h"

#include <QApplication>
#ifdef _WIN32
#include <Windows.h>
#endif

int ParseArgs(FString& targetProj)
{
	int r = 0;
	const auto& args = FCommandLine::GetArgs();
	if (args.Size() > 0)
	{
		if (SizeType i = args[0].Find(".thproj"); i != -1)
		{
			FKeyValue kv(args[0], EKeyValueType::KV_STANDARD_ASCII);
			if (auto* v = kv.GetValue("engine_version", false); v != nullptr)
			{
				if (v->Value != ENGINE_VERSION)
				{
					WString enginePath = CEngine::OSGetEnginePath(v->Value);
					THORIUM_ASSERT(!enginePath.IsEmpty(), "The engine version this project requires is not installed on this computer!");

					WString args = L"";
#ifdef _WIN32
					STARTUPINFO si{};
					PROCESS_INFORMATION pi{};

					si.cb = sizeof(si);

					CreateProcess((enginePath + L"\\bin\\Thorium Sdk.exe").c_str(), (wchar_t*)args.c_str(), nullptr, nullptr, false, CREATE_NEW_PROCESS_GROUP, nullptr, nullptr, &si, &pi);
#endif
					return 1;
				}
				targetProj = args[0];
				targetProj.Erase(targetProj.begin() + targetProj.FindLastOf("/\\"), targetProj.end());
			}

		}
	}

	for (SizeType i = 0; i < args.Size(); i++)
	{
		const FString& arg = args[i];
		if (arg == "-target" && i + 1 < args.Size())
		{
			i++;
			targetProj = args[i];
			continue;
		}
		
	}

	if (targetProj.IsEmpty())
	{
		targetProj = "..\\.project";
	}

	return r;
}

int Startup()
{
	FString targetProject;
	if (ParseArgs(targetProject) != 0)
		return 0;
	
	bool bLoadEditor = false;
	if (FFileHelper::DirectoryExists(targetProject))
	{
		bLoadEditor = true;
#ifdef _WIN32
		SetCurrentDirectory(ToWString(targetProject + "\\..\\").c_str());
#endif
	}

	gIsEditor = true;

	CToolsWindow* wnd;

	if (bLoadEditor)
		wnd = CToolsWindow::Create<CEditorWindow>();
	else
		wnd = CToolsWindow::Create<CProjectManagerWnd>();

	return ToolsApp->exec();
}

int main(int argc, char** argv)
{
#ifdef _WIN32
#ifdef _DEBUG
	if (!IsDebuggerPresent())
		MessageBoxA(nullptr, "", "No debugger attached", MB_OK);
#endif
#endif
	FCommandLine::Parse(argv, argc);

	//QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
	ToolsApp = new QApplication(argc, argv);

	return Startup();
}
