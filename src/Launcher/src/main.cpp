
#include <string>
#include <Util/Core.h>
#include <Util/Assert.h>
#include <Util/KeyValue.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <dlfcn.h>
#include <fstream>

#include <iostream>
#endif

#ifdef _WIN32
#include "iomanip"
#include <sstream>
#include "minidumpapiset.h"

LONG WINAPI Win32ExceptionHandler(_EXCEPTION_POINTERS* exceptionInfo)
{
	typedef BOOL(WINAPI* MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType, CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

	HMODULE mhlib = LoadLibrary("dbghelp.dll");
	MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)GetProcAddress(mhlib, "MiniDumpWriteDump");

	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);

	std::ostringstream oss;
	oss << std::put_time(&tm, "%d-%m-%y %H-%M-%S");
	std::string timeTxt = oss.str();

	_MINIDUMP_EXCEPTION_INFORMATION ExInfo;
	ExInfo.ThreadId = ::GetCurrentThreadId();
	ExInfo.ExceptionPointers = exceptionInfo;
	ExInfo.ClientPointers = FALSE;
	
	HANDLE hFile = CreateFile(("crash " + timeTxt + ".dmp").c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	pDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpWithFullMemory, &ExInfo, NULL, NULL);
	CloseHandle(hFile);
	return EXCEPTION_CONTINUE_SEARCH;
}
#endif

FString GetEnginePath(const FString& version)
{
#ifdef _WIN32
	FString keyPath = "SOFTWARE\\ThoriumEngine\\" + version;

	HKEY hKey;
	LONG lRes = RegOpenKeyExA(HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_READ, &hKey);
	if (lRes == ERROR_FILE_NOT_FOUND)
		return "";

	CHAR strBuff[MAX_PATH];
	DWORD buffSize = sizeof(strBuff);
	lRes = RegQueryValueExA(hKey, "path", 0, NULL, (LPBYTE)strBuff, &buffSize);
	if (lRes != ERROR_SUCCESS)
		return "";

	return strBuff;
#else
	std::ifstream stream(std::string(getenv("HOME")) + "/.thoriumengine/" + version.c_str() + "/path.txt", std::ios_base::in);
	if (!stream.is_open())
	{
		return FString();
	}

	std::string str;
	std::getline(stream, str);

	return str.c_str();
#endif
}

#if _WIN32
#define THOpenLib(file) LoadLibrary(file)
#else
#define THOpenLib(file) dlopen(file, RTLD_NOW)
#endif

#ifdef _WIN32
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
#else
int main(int argc, char** argv)
#endif
{
#if _WIN32
	SetUnhandledExceptionFilter(Win32ExceptionHandler);
#endif

	bool bForceLocalEngine = false;
#if _WIN32
	TArray<FString> args = FString(lpCmdLine).Split(' ');
	for (auto& arg : args)
	{
#else
	for (int i = 0; i < argc; i++)
	{
		FString arg = argv[i];
#endif

		if (arg == "-forceLocalEngine")
			bForceLocalEngine = true;
	}

	// Preload EngineDll
#if _WIN32
	HMODULE EngineLib = LoadLibrary("core\\bin\\win64\\Engine.dll");
#else
	void* EngineLib = dlopen("core/bin/linux/Engine.so", RTLD_NOW);
#endif
	FString engineFile;
	FString enginePath;
	if (!EngineLib && !bForceLocalEngine)
	{
		FString engineVersion = "1.0";
		FKeyValue kv("config/project.cfg");
		if (kv.IsOpen())
			engineVersion = kv.GetValue("engine_version")->Value;

		enginePath = GetEnginePath(engineVersion);
#if _WIN32
		engineFile = enginePath + "\\bin\\win64\\Engine.dll";
		EngineLib = LoadLibrary((enginePath + "\\bin\\win64\\Engine.dll").c_str());
#else
		engineFile = enginePath + "/bin/linux/Engine.so";
		EngineLib = dlopen((enginePath + "/bin/linux/Engine.so").c_str(), RTLD_NOW);
#endif
	}

#ifndef _WIN32
	if (!EngineLib)
		std::cerr << dlerror() << std::endl;
#endif

	UTIL_ASSERT(EngineLib, "Unable to load '" + engineFile + "'!");

	// Load Dependencies
	

	// Load LauncherDll
#if _WIN32
	HMODULE launcher = LoadLibrary((enginePath + "\\bin\\win64\\Launcher.dll").c_str());
#else
	void* launcher = dlopen((enginePath + "/bin/linux/Launcher.so").c_str(), RTLD_NOW);
#endif
	if (!launcher)
	{
		std::string err = "Unable to load 'launcher.dll' -";
#if _WIN32
		err += std::to_string(GetLastError());
		MessageBox(NULL, err.c_str(), "Error", MB_OK);
#else
		std::cerr << "error: failed to load launcher.so!\n";
#endif
		return -1;
	}

	typedef int(*LaunchFunc)(const char*);
#if _WIN32
	LaunchFunc _launch = (LaunchFunc)GetProcAddress(launcher, "Launch");
#else
	LaunchFunc _launch = (LaunchFunc)dlsym(launcher, "Launch");

	FString cmdLine;
	for (int i = 0; i < argc; i++)
	{
		cmdLine += argv[i];
		if (i != argc - 1)
			cmdLine += ' ';
	}
	const char* lpCmdLine = cmdLine.c_str();
#endif

	if (!_launch)
	{
		std::string err = "Couldn't find function 'int Launch(const char*)' in Launcher.dll -";
#if _WIN32
		err += std::to_string(GetLastError());
		MessageBox(NULL, err.c_str(), "Error", MB_OK);
#else
		std::cerr << "error: Couldn't find function 'int Launch(const char*)' in Launcher.so!\n";
#endif
		return -1;
	}

	//SetCurrentDirectory("..\\");

	//MessageBox(NULL, "Hello :)", "Error", MB_OK);

	return _launch(lpCmdLine);
}
