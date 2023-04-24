#pragma once

#include "EngineCore.h"
#include <functional>
#include <Util/Event.h>

#define CONSOLE_USE_ARRAY 1

enum EConsoleMsgType
{
	CONSOLE_PLAIN,
	CONSOLE_INFO,
	CONSOLE_WARNING,
	CONSOLE_ERROR
};

struct FConsoleMsgInfo
{
	long line;
	const char* file;
	const char* function;
};

struct FConsoleMsg
{
	FString msg;
	FConsoleMsgInfo info;
	EConsoleMsgType type;
	FString module;

	FConsoleMsg* next = nullptr;
};

class ENGINE_API CConCmd
{
	typedef void(*CmdFuncPtr)(const TArray<FString>& args);
	typedef void(*CmdFuncPtrNoArgs)();

public:
	CConCmd(const FString& name, CmdFuncPtr func);
	CConCmd(const FString& name, CmdFuncPtrNoArgs func);
	~CConCmd();

	virtual void Exec(const TArray<FString>& args);

	inline const FString& Name() const { return name; }

private:
	FString name;
	std::function<void(const TArray<FString>&)> func;
};

class ENGINE_API CConVar
{
public:
	CConVar(const FString& name, float value = 0.f);
	explicit CConVar(const FString& name, const WString& configPath, float value = 0.f);
	~CConVar();

	float AsFloat() const { return value; }
	int AsInt() const { return (int)value; }
	bool AsBool() const { return value != 0.f; }
	
	void SetValue(float v) { value = v; }
	void SetValue(int v) { value = (float)v; }
	void SetValue(bool v) { value = (float)v; }

	inline const FString& Name() const { return name; }

	// Returns the config path. 
	// If set, during project initialization this variable will be loaded from the specified file.
	inline const WString& ConfigPath() const { return configPath; }

private:
	// Config path relative to project dir.
	WString configPath;

	FString name;
	float value;
};

class ENGINE_API CConsole
{
	friend class CConCmd;
	friend class CConVar;

public:
	static void Init();
	static void Shutdown();

	static void LoadConfig();

public:
	static inline void LogPlain(const FString& msg, FConsoleMsgInfo info = {}) { _log({ msg, info, CONSOLE_PLAIN, FString(), nullptr }); }
	static inline void LogInfo(const FString& msg, const FString& module, FConsoleMsgInfo info = {}) { _log({ msg, info, CONSOLE_INFO, module, nullptr }); }
	static inline void LogWarning(const FString& msg, const FString& module, FConsoleMsgInfo info = {}) { _log({ msg, info, CONSOLE_WARNING, module, nullptr }); }
	static inline void LogError(const FString& msg, const FString& module, FConsoleMsgInfo info = {}) { _log({ msg, info, CONSOLE_ERROR, module, nullptr }); }

	static void Exec(const FString& input);

#if CONSOLE_USE_ARRAY
	static const TArray<FConsoleMsg>& GetMsgCache();
#else
	static FConsoleMsg* GetLinkedList();
#endif

	static inline TEvent<const FConsoleMsg&>& GetLogEvent() { return onMsgLogged; }

	inline static const TArray<CConVar*>& GetConVars() { return consoleVars; }
	inline static const TArray<CConCmd*>& GetConCmds() { return consoleCmds; }

	static CConVar* GetConVar(const FString& name);

private:
	static void _log(const FConsoleMsg& msg);

private:
	static TEvent<const FConsoleMsg&> onMsgLogged;
	static TArray<CConVar*> consoleVars;
	static TArray<CConCmd*> consoleCmds;

};

#define CONSOLE_LogInfo(module, msg) CConsole::LogInfo(msg, module, { __LINE__, __FILE__, __FUNCTION__ })
#define CONSOLE_LogWarning(module, msg) CConsole::LogWarning(msg, module, { __LINE__, __FILE__, __FUNCTION__ })
#define CONSOLE_LogError(module, msg) CConsole::LogError(msg, module, { __LINE__, __FILE__, __FUNCTION__ })
