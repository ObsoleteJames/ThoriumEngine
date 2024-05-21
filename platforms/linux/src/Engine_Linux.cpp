
#include "Engine.h"

#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>

FString CEngine::OSGetEnginePath(const FString& engineVersion)
{
	std::ifstream stream(std::string(getenv("HOME")) + "/.thoriumengine/" + version.c_str() + "/path.txt", std::ios_base::in);
	if (!stream.is_open())
	{
		return FString();
	}

	std::string str;
	std::getline(stream, str);

	return str.c_str();
}

FString CEngine::OSGetDataPath()
{
	return FString(getenv("HOME")) + "/.thoriumengine/" + version.c_str();
}

FString CEngine::OSGetDocumentsPath()
{
	return FString(getenv("HOME")) + "/Documents";
}

FString CEngine::OpenFileDialog(const FString& filter /*= FString()*/)
{
	return FString();
}

FString CEngine::SaveFileDialog(const FString& filter /*= FString()*/)
{
	return FString();
}

FString CEngine::OpenFolderDialog()
{
	return FString();
}

extern char** environ;

int CEngine::ExecuteProgram(const FString& cmd, bool bWait)
{
	TArray<FString> args = cmd.Split(" \t");
	FString exec = args[0];
	args.Erase(args.first()); 
	
	TArray<const char*> args_c(args.Size() + 1);
	for (int i = 0; i < args.size(); i++)
		args_c[i] = args[i].c_str();
	
	args_c.Add(0);

	pid_t pid;
	int status = posix_spawn(&pid, exec.c_str(), NULL, NULL, (char**)args_c.Data(), environ);

	if (status != 0)
		return status;

	if (bWait)
		waitpid(pid, &status, 0);

	return status;
}
