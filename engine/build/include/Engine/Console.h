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
	SizeType time;

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
	enum EConvarAuthority
	{
		SERVER_ONLY, // Only the server can change this.
		SERVER_CHEAT, // Can only be changed if singleplayer, or server has cheats enabled.
		CLIENT, // Client only convar.
	};

public:
	CConVar(const FString& name, float value = 0.f, float min = 0.f, float max = 0.f, EConvarAuthority auth = CLIENT);
	CConVar(const FString& name, float value, EConvarAuthority auth, float min = 0.f, float max = 0.f);
	explicit CConVar(const FString& name, const FString& configPath, float value = 0.f, float min = 0.f, float max = 0.f, EConvarAuthority auth = CLIENT);
	explicit CConVar(const FString& name, const FString& configPath, float value, EConvarAuthority auth, float min = 0.f, float max = 0.f);
	~CConVar();

	float AsFloat() const { return value; }
	int AsInt() const { return (int)value; }
	bool AsBool() const { return value != 0.f; }
	
	void SetValue(float v);
	void SetValue(int v);
	void SetValue(bool v);

	inline const FString& Name() const { return name; }

	inline EConvarAuthority Authority() const { return authority; }

	// Returns the config path. 
	// If set, during project initialization this variable will be loaded from the specified file.
	inline const FString& ConfigPath() const { return configPath; }

public:
	TEvent<float> onValueChanged;

protected:
	void Register();

private:
	// Config path relative to project dir.
	FString configPath;

	FString name;
	
	EConvarAuthority authority = CLIENT;

	float min, max;
	float value;
};

class ENGINE_API CConsole
{
	friend class CConCmd;
	friend class CConVar;

public:
	static void Init();
	static void Update();
	static void Shutdown();

	static void LoadConfig();

	static void EnableStdio();

public:
	static inline void LogPlain(const FString& msg, FConsoleMsgInfo info = {}) { _log({ msg, info, CONSOLE_PLAIN, FString(), 0, nullptr }); }
	static inline void LogInfo(const FString& msg, const FString& module, FConsoleMsgInfo info = {}) { _log({ msg, info, CONSOLE_INFO, module, 0, nullptr }); }
	static inline void LogWarning(const FString& msg, const FString& module, FConsoleMsgInfo info = {}) { _log({ msg, info, CONSOLE_WARNING, module, 0, nullptr }); }
	static inline void LogError(const FString& msg, const FString& module, FConsoleMsgInfo info = {}) { _log({ msg, info, CONSOLE_ERROR, module, 0, nullptr }); }

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

	static void _logCout(const FConsoleMsg& msg);

private:
	static TEvent<const FConsoleMsg&> onMsgLogged;
	static TArray<CConVar*> consoleVars;
	static TArray<CConCmd*> consoleCmds;

};

#define CONSOLE_LogInfo(module, msg) CConsole::LogInfo(msg, module, { __LINE__, __FILE__, __FUNCTION__ })
#define CONSOLE_LogWarning(module, msg) CConsole::LogWarning(msg, module, { __LINE__, __FILE__, __FUNCTION__ })
#define CONSOLE_LogError(module, msg) CConsole::LogError(msg, module, { __LINE__, __FILE__, __FUNCTION__ })
