
#include "FileHelper.h"

#include <filesystem>
#include <sys/types.h>
#include <sys/stat.h>

namespace fs = std::filesystem;

bool FFileHelper::DirectoryExists(const FString& path)
{
	struct stat info;

	if (stat(path.c_str(), &info) != 0)
		return false;
	if (info.st_mode & S_IFDIR)
		return true;

	return false;
}

bool FFileHelper::FileExists(const FString& path)
{
	struct stat info;

	if (stat(path.c_str(), &info) != 0)
		return false;
	if (info.st_mode & S_IFDIR)
		return false;

	return true;
}

bool FFileHelper::DirectoryExists(const WString& path)
{
	if (fs::exists(path.c_str()))
		return true;

	return false;
}

bool FFileHelper::FileExists(const WString& path)
{
	if (fs::exists(path.c_str()))
		return true;

	return false;
}
