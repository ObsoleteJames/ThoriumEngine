
#include "Registry/FileSystem.h"
#include <unistd.h>

void CFileSystem::SetCurrentPath(const FString& path)
{
	chdir(path.c_str());
}

FString CFileSystem::GetCurrentPath()
{
	char buff[128];

	getcwd(buff, 128);
	return buff;
}
