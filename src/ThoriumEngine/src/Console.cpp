
#include "Console.h"
#include <Util/Assert.h>
#include <mutex>
#include <string>

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
				CONSOLE_LogWarning("CConsole", "Insufficient arguments, expected 1 but got 0");
				return;
			}

			bool bValidArg = args[0].IsNumber();
			bool bIsBool = args[0] == "true" || args[0] == "false";
			if (!bValidArg && !bIsBool)
			{
				CONSOLE_LogWarning("CConsole", FString("Invalid input value '") + args[0] + "'");
				return;
			}

			_log({ target + " = " + args[0], {0, "", ""}, CONSOLE_PLAIN, FString(), nullptr});

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

void CConsole::_log(const FConsoleMsg& msg)
{
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

	onMsgLogged.Fire(msg);
	consoleMutex.unlock();
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

CConVar::CConVar(const FString& n, float v) : value(v), name(n)
{
	for (auto* cmd : CConsole::consoleCmds)
		THORIUM_ASSERT(cmd->Name() != name, FString("Failed to register ConVar '") + name + "', ConCmd with the same name already exists!");
	for (auto* var : CConsole::consoleVars)
		THORIUM_ASSERT(var->Name() != name, FString("Failed to register ConVar '") + name + "', ConVar with the same name arleady exists!");

	CConsole::consoleVars.Add(this);
}

CConVar::~CConVar()
{
	auto it = CConsole::consoleVars.Find(this);
	if (it != CConsole::consoleVars.end())
		CConsole::consoleVars.Erase(it);
}
