
#include <string>
#include <Util/Core.h>
#include <Util/Assert.h>
#include <Util/KeyValue.h>

#ifdef _WIN32
#include <windows.h>
#endif

FString GetEnginePath(const FString& version)
{
#ifdef _WIN32
	WString engineVersion = ToWString(version);
	WString keyPath = WString(L"SOFTWARE\\ThoriumEngine\\") + engineVersion;

	HKEY hKey;
	LONG lRes = RegOpenKeyExW(HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_READ, &hKey);
	if (lRes == ERROR_FILE_NOT_FOUND)
		return "";

	WCHAR strBuff[MAX_PATH];
	DWORD buffSize = sizeof(strBuff);
	lRes = RegQueryValueExW(hKey, L"path", 0, NULL, (LPBYTE)strBuff, &buffSize);
	if (lRes != ERROR_SUCCESS)
		return "";

	return ToFString(WString(strBuff));
#endif
}

#ifdef _WIN32
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	// Preload EngineDll
	HMODULE EngineLib = LoadLibrary(".\\bin\\Engine.dll");
	FString enginePath = ".";
	if (!EngineLib)
	{
		FString engineVersion;
		FKeyValue kv(L"config\\project.cfg");
		if (kv.IsOpen())
		{
			engineVersion = kv.GetValue("engine_version")->Value;

			enginePath = GetEnginePath(engineVersion);
			EngineLib = LoadLibrary((enginePath + "\\bin\\Engine.dll").c_str());
		}
	}

	UTIL_ASSERT(EngineLib, "Unable to load 'engine.dll'!");

	// Load Dependencies
	

	// Load LauncherDll
	HMODULE launcher = LoadLibrary((enginePath + "\\bin\\Launcher.dll").c_str());
	if (!launcher)
	{
		std::string err = "Unable to load 'launcher.dll' -";
		err += std::to_string(GetLastError());
		MessageBox(NULL, err.c_str(), "Error", MB_OK);
		return -1;
	}

	typedef int(*LaunchFunc)(const char*);
	LaunchFunc _launch = (LaunchFunc)GetProcAddress(launcher, "Launch");
	if (!_launch)
	{
		std::string err = "Couldn't find function 'int Launch(const char*)' in Launcher.dll -";
		err += std::to_string(GetLastError());
		MessageBox(NULL, err.c_str(), "Error", MB_OK);
		return -1;
	}

	//SetCurrentDirectory("..\\");

	MessageBox(NULL, "Hello :)", "Error", MB_OK);

	return _launch(lpCmdLine);
}
#endif
