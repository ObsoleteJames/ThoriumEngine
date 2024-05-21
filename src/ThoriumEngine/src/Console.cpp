
#define UTIL_STD_STRING
#include <string>
#include "Console.h"
#include "Math/Math.h"
#include <Util/Assert.h>
#include <Util/KeyValue.h>
#include <mutex>

#include <iostream>

#define CONSOLE_MAX 512

static std::mutex consoleMutex;

#if CONSOLE_USE_ARRAY
static TArray<FConsoleMsg> logArray;
#else
static FConsoleMsg* logArray;
#endif
static SizeType beginIndex;
static SizeType endIndex;
static SizeType numLogs;

static bool bPrintToIO = false;

TEvent<const FConsoleMsg&> CConsole::onMsgLogged;
TArray<CConVar*> CConsole::consoleVars;
TArray<CConCmd*> CConsole::consoleCmds;

void CConsole::Init()
{
	//logArray = (FConsoleMsg*)malloc(sizeof(FConsoleMsg) * CONSOLE_MAX);
#if !CONSOLE_USE_ARRAY
	logArray = new FConsoleMsg[CONSOLE_MAX];
#else
	logArray.Reserve(64);
#endif

	beginIndex = 0;
	endIndex = 0;
}

void CConsole::Shutdown()
{
#if !CONSOLE_USE_ARRAY
	delete[] logArray;
#endif

	// Save Config.
	for (auto& var : consoleVars)
	{
		if (var->ConfigPath().IsEmpty())
			continue;

		FKeyValue kv(var->ConfigPath());
		kv.GetValue(var->Name())->Value = std::to_string(var->AsFloat()).c_str();

		kv.Save();
	}
}

void CConsole::LoadConfig()
{
	for (auto& var : consoleVars)
	{
		if (var->ConfigPath().IsEmpty())
			continue;

		FKeyValue kv(var->ConfigPath());
		if (!kv.IsOpen())
			continue;

		const FString& v = kv.GetValue(var->Name())->Value;
		if (!v.IsEmpty() && v.IsNumber())
			var->SetValue(std::stof(v.c_str()));
	}
}

void CConsole::EnableStdio()
{
	bPrintToIO = true;
}

void CConsole::Exec(const FString& input)
{
	TArray<FString> args = input.Split(' ');
	FString target = args[0];
	args.Erase(args.begin());

	for (auto* cmd : consoleCmds)
	{
		if (cmd->Name() == target)
		{
			cmd->Exec(args);

			//FConsoleMsg msg;
			//msg.type = CONSOLE_PLAIN;
			//msg.msg = target;
			//_log(msg);
			return;
		}
	}
	for (auto* cmd : consoleVars)
	{
		if (cmd->Name() == target)
		{
			if (args.Size() == 0 || args[0].IsEmpty())
			{
				//CONSOLE_LogWarning("CConsole", "Insufficient arguments, expected 1 but got 0");

				_log({ target + ": " + (FString)std::to_string(cmd->AsFloat()), {0, "", ""}, CONSOLE_PLAIN, FString(), 0, nullptr });
				return;
			}

			bool bValidArg = args[0].IsNumber();
			bool bIsBool = args[0] == "true" || args[0] == "false";
			if (!bValidArg && !bIsBool)
			{
				CONSOLE_LogWarning("CConsole", FString("Invalid input value '") + args[0] + "'");
				return;
			}

			_log({ target + " = " + args[0], {0, "", ""}, CONSOLE_PLAIN, FString(), 0, nullptr});

			float f = 0.f;
			if (!bIsBool)
				f = std::stof(args[0].c_str());
			else
				f = args[0] == "true" ? 1.f : 0.f;

			cmd->SetValue(f);
			return;
		}
	}

	CONSOLE_LogWarning("CConsole", FString("Unknown command '") + target + "'");
}

#if CONSOLE_USE_ARRAY
const TArray<FConsoleMsg>& CConsole::GetMsgCache()
{
	return logArray;
}
#else
FConsoleMsg* CConsole::GetLinkedList()
{
	return &logArray[beginIndex];
}
#endif

CConVar* CConsole::GetConVar(const FString& name)
{
	for (auto* var : consoleVars)
		if (var->Name() == name)
			return var;

	return nullptr;
}

void CConsole::_log(const FConsoleMsg& _msg)
{
	FConsoleMsg msg = _msg;
	msg.time = (SizeType)time(nullptr);

	consoleMutex.lock();
#if !CONSOLE_USE_ARRAY
	if (endIndex == beginIndex && numLogs > 0)
	{
		beginIndex++;
		beginIndex %= CONSOLE_MAX;
	}

	SizeType index = endIndex;
	endIndex++;
	endIndex %= CONSOLE_MAX;

	new (&logArray[index]) FConsoleMsg(msg);
	//logArray[index] = msg;
	logArray[(CONSOLE_MAX + (index - 1))  % CONSOLE_MAX].next = &logArray[index];

	if (numLogs != CONSOLE_MAX)
		numLogs++;
#else
	logArray.Add(msg);
#endif

	if (bPrintToIO)
		_logCout(msg);

	onMsgLogged.Fire(msg);
	consoleMutex.unlock();
}

static FString TimeToHmsString(time_t* time)
{
	struct tm time_info;
	char timeString[9];  // space for "HH:MM:SS\0"

#if _WIN32
	localtime_s(&time_info, time);
#else
	time_info = *localtime(time);
#endif

	strftime(timeString, sizeof(timeString), "%H:%M:%S", &time_info);
	timeString[8] = '\0';
	return FString(timeString);
}

void CConsole::_logCout(const FConsoleMsg &msg)
{
	const char* txtCol;
	if (msg.type == CONSOLE_PLAIN)
		txtCol = "\033[0;40m\033[0;90m";
	else if (msg.type == CONSOLE_INFO)
		txtCol = "\033[0;40m\033[0;37m";
	else if (msg.type == CONSOLE_WARNING)
		txtCol = "\033[1;40m\033[0;33m";
	else if (msg.type == CONSOLE_ERROR)
		txtCol = "\033[1;41m\033[0;37m";

	FString timeTxt = TimeToHmsString((time_t*)&msg.time);

	std::cout << "\033[0;40m\033[0;32m[" << timeTxt.c_str() << "] " << msg.module.c_str() << " " << txtCol << msg.msg.c_str() << "\n";
}

CConCmd::CConCmd(const FString& n, CmdFuncPtr f) : name(n)
{
	for (auto* cmd : CConsole::consoleCmds)
		THORIUM_ASSERT(cmd->Name() != name, FString("Failed to register ConCmd '") + name + "', command with the same name already exists!");
	for (auto* var : CConsole::consoleVars)
		THORIUM_ASSERT(var->Name() != name, FString("Failed to register ConCmd '") + name + "', ConVar with the same name arleady exists!");

	func = f;
	CConsole::consoleCmds.Add(this);
}

CConCmd::CConCmd(const FString& n, CmdFuncPtrNoArgs f) : name(n)
{
	for (auto* cmd : CConsole::consoleCmds)
		THORIUM_ASSERT(cmd->Name() != name, FString("Failed to register ConCmd '") + name + "', command with the same name already exists!");
	for (auto* var : CConsole::consoleVars)
		THORIUM_ASSERT(var->Name() != name, FString("Failed to register ConCmd '") + name + "', ConVar with the same name arleady exists!");

	func = [=](const TArray<FString>&) { f(); };
	CConsole::consoleCmds.Add(this);
}

CConCmd::~CConCmd()
{
	auto it = CConsole::consoleCmds.Find(this);
	if (it != CConsole::consoleCmds.end())
		CConsole::consoleCmds.Erase(it);
}

void CConCmd::Exec(const TArray<FString>& args)
{
	func(args);
}

CConVar::CConVar(const FString& n, float v, float _min, float _max, EConvarAuthority auth) : value(v), name(n), min(_min), max(_max), authority(auth)
{
	Register();
}

CConVar::CConVar(const FString& n, const FString& c /*= FString()*/, float v /*= 0.f*/, float _min, float _max, EConvarAuthority auth) : value(v), name(n), configPath(c), min(_min), max(_max), authority(auth)
{
	Register();
}

CConVar::CConVar(const FString& n, const FString& c, float v, EConvarAuthority auth, float _min /*= 0.f*/, float _max /*= 0.f*/) : value(v), name(n), configPath(c), min(_min), max(_max), authority(auth)
{
	Register();
}

CConVar::CConVar(const FString& n, float v, EConvarAuthority auth, float _min /*= 0.f*/, float _max /*= 0.f*/) : value(v), name(n), min(_min), max(_max), authority(auth)
{
	Register();
}

CConVar::~CConVar()
{
	auto it = CConsole::consoleVars.Find(this);
	if (it != CConsole::consoleVars.end())
		CConsole::consoleVars.Erase(it);
}

void CConVar::SetValue(float v)
{
	// TODO: Check Authority.

	value = v;

	if (min < max)
		value = FMath::Clamp(value, min, max);

	onValueChanged.Fire(value);
}

void CConVar::SetValue(int v)
{
	value = (float)v;

	if (min < max)
		value = FMath::Clamp(value, min, max);

	onValueChanged.Fire(value);
}

void CConVar::SetValue(bool v)
{
	value = (float)v;

	if (min < max)
		value = FMath::Clamp(value, min, max);

	onValueChanged.Fire(value);
}

void CConVar::Register()
{
	for (auto* cmd : CConsole::consoleCmds)
		THORIUM_ASSERT(cmd->Name() != name, FString("Failed to register ConVar '") + name + "', ConCmd with the same name already exists!");
	for (auto* var : CConsole::consoleVars)
		THORIUM_ASSERT(var->Name() != name, FString("Failed to register ConVar '") + name + "', ConVar with the same name arleady exists!");

	CConsole::consoleVars.Add(this);
}
