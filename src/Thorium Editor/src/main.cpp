
#include <Util/KeyValue.h>
#include "EditorEngine.h"
#include "Misc/CommandLine.h"

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
			FKeyValue kv(ToWString(args[0]), EKeyValueType::KV_STANDARD_ASCII);
			if (auto* v = kv.GetValue("engine_version", false); v != nullptr)
			{
				if (v->Value != ENGINE_VERSION)
				{
					WString enginePath = CEngine::OSGetEnginePath(v->Value);
					THORIUM_ASSERT(!enginePath.IsEmpty(), "The engine version this project requires is not install on this computer.");

					WString args = L"";

#ifdef _WIN32
					STARTUPINFO si{};
					PROCESS_INFORMATION pi{};

					si.cb = sizeof(si);
					CreateProcess((enginePath + L"\\bin\\Thorium Editor.exe").c_str(), (wchar_t*)args.c_str(), nullptr, nullptr, false, CREATE_NEW_PROCESS_GROUP, nullptr, nullptr, &si, &pi);
#endif
					return 1;
				}
				targetProj = args[0];
				targetProj.Erase(targetProj.begin() + targetProj.FindLastOf("/\\"), targetProj.end());
			}
		}
	}

	if (targetProj.IsEmpty())
	{
		targetProj = "..\\.project";
	}
	return r;
}

#ifdef _WIN32
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
#else
int main(int argc, char** argv)
#endif
{
#ifdef _WIN32
#ifdef _DEBUG
	if (!IsDebuggerPresent())
		MessageBoxA(nullptr, "", "No debugger attached", MB_OK);
#endif
#endif

#ifdef _WIN32
	FCommandLine::Parse(lpCmdLine);
#else
	FCommandLine::Parse(argv, argc);
#endif

	int exitCode = 0;

	FString project;
	if (ParseArgs(project) != 0)
		return 0;

	gEngine = new CEditorEngine();
	gEngine->Init();

	gEngine->LoadProject(ToWString(project) + L"\\..");

	exitCode = gEngine->Run();
	return exitCode;
}
