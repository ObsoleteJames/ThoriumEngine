
#include <Util/Assert.h>
#include <string>
#include "CommandLine.h"

//static FString CmdLine;
ENGINE_API TArray<FString> FCommandLine::CmdLine;

void FCommandLine::Parse(const char* in_cmd, bool bIgnoreFirst)
{
	//if (CmdLine.Capacity() < COMMANDLINE_SIZE)
	//	CmdLine.Reserve(COMMANDLINE_SIZE);

	if (bIgnoreFirst)
	{
		while (*in_cmd != ' ')
		{
			if (*in_cmd == '\0')
				return;

			in_cmd++;
		}
		in_cmd++; // Make sure that we don't stay on the space.
	}

	CmdLine.Add(FString());
	bool bInQuotes = false;
	while (*in_cmd)
	{
		if (*in_cmd == '"')
		{
			bInQuotes ^= 1;
			in_cmd++;
			continue;
		}

		if (!bInQuotes && *in_cmd == ' ')
		{
			CmdLine.Add(FString());
			in_cmd++;
			continue;
		}

		(*CmdLine.last()) += (char)(*in_cmd);
		in_cmd++;
	}
}

void FCommandLine::Parse(const wchar_t* in_cmd, bool bIgnoreFirst)
{
	//if (CmdLine.Capacity() < COMMANDLINE_SIZE)
	//	CmdLine.Reserve(COMMANDLINE_SIZE);

	if (bIgnoreFirst)
	{
		while (*in_cmd != ' ')
			in_cmd++;
		in_cmd++; // Make sure that we don't stay on the space.
	}

	CmdLine.Add(FString());
	bool bInQuotes = false;
	while (*in_cmd)
	{
		if (*in_cmd == '"')
		{
			bInQuotes ^= 1;
			in_cmd++;
			continue;
		}

		if (!bInQuotes && *in_cmd == ' ')
		{
			CmdLine.Add(FString());
			in_cmd++;
			continue;
		}

		(*CmdLine.last()) += (char)(*in_cmd);
		in_cmd++;
	}
}

void FCommandLine::Parse(char** in_cmd, int argc, bool bIngoreFirst /*= true*/)
{
	for (int i = bIngoreFirst ? 1 : 0; i < argc; i++)
		CmdLine.Add(FString(in_cmd[i]));
}

void FCommandLine::Append(const FString& str)
{
	THORIUM_ASSERT((CmdLine.Size() + 1) > COMMANDLINE_SIZE, "CmdLine out of space.");

	CmdLine.Add(str);
}

SizeType FCommandLine::FindParam(const FString& str)
{
	for (int i = 0; i < CmdLine.Size(); i++)
	{
		if (CmdLine[i] == str)
			return i;
	}

	return -1;
}

FString FCommandLine::ParamValue(const FString& str)
{
	SizeType index = FindParam(str);
	if (index == -1 || index + 1 >= CmdLine.Size())
		return "";

	return CmdLine[index + 1];
}

int FCommandLine::ParamValue(const FString& str, int defaultVal /*= 0*/)
{
	SizeType index = FindParam(str);
	if (index == -1 || index + 1 >= CmdLine.Size())
		return defaultVal;

	FString r = CmdLine[index + 1];
	if (!r.IsNumber())
		return defaultVal;

	return std::stoi(r.c_str());
}

float FCommandLine::ParamValue(const FString& str, float defaultVal /*= 0.f*/)
{
	SizeType index = FindParam(str);
	if (index == -1 || index + 1 >= CmdLine.Size())
		return defaultVal;

	FString r = CmdLine[index + 1];
	if (!r.IsNumber())
		return defaultVal;

	return std::stof(r.c_str());
}
