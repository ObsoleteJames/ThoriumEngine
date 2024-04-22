
#include <string>
#include "FileSystem.h"
#include "Engine.h"
#include "Misc/FileHelper.h"
#include "Resources/ResourceManager.h"
#include "Console.h"
#include <filesystem>

TArray<FMod*> CFileSystem::Mods;
namespace fs = std::filesystem;

FDirectory* FMod::FindDirectory(const FString& path) const
{
	const char* pathPtr = path.c_str();

	const FDirectory* dir = &root;
	FString target;
	while (dir)
	{
		bool bIsEnd = false;
		target.Clear();

		while (true)
		{
			if (pathPtr[0] == '\0')
			{
				bIsEnd = true;
				break;
			}
			if (pathPtr[0] == '\\' || pathPtr[0] == '/')
			{
				pathPtr++;
				break;
			}
			target += pathPtr[0];
			pathPtr++;
		}

		FDirectory* newDir = nullptr;
		for (auto& d : dir->directories)
		{
			if (d->name == target)
			{
				if (bIsEnd)
					return d;

				newDir = d;
				break;
			}
		}

		dir = newDir;
	}
	return nullptr;
}

FDirectory* FMod::CreateDir(const FString& path)
{
	const char* pathPtr = path.c_str();

	if (CFileSystem::IsBlacklisted(path))
		return nullptr;

	if (pathPtr[0] == '\\' || pathPtr[0] == '/')
		pathPtr++;

	FDirectory* dir = &root;
	FString target;
	while (dir)
	{
		bool bIsEnd = false;
		target.Clear();

		while (true)
		{
			if (pathPtr[0] == '\0')
			{
				bIsEnd = true;
				break;
			}
			if (pathPtr[0] == '\\' || pathPtr[0] == '/')
			{
				pathPtr++;
				break;
			}
			target += pathPtr[0];
			pathPtr++;
		}

		FDirectory* newDir = nullptr;
		for (auto& d : dir->directories)
		{
			if (d->name == target)
			{
				newDir = d;
				break;
			}
		}

		if (newDir == nullptr)
		{
			dir->directories.Add(new FDirectory());
			newDir = *dir->directories.last();

			newDir->name = target;
			newDir->parent = dir;
		}

		dir = newDir;

		if (bIsEnd)
			break;
	}

	fs::create_directories((this->path + "/" + path).c_str());
	if (gIsEditor && HasSdkContent())
		fs::create_directories((sdkPath + "/" + path).c_str());
	return dir;
}

void FMod::DeleteFile(const FString& path)
{
	FFile* f = FindFile(path);
	if (!f)
		return;

	std::filesystem::remove(f->FullPath().c_str());

	FDirectory* dir = f->Dir();
	auto it = dir->files.Find(f);
	if (it != dir->files.end())
		dir->files.Erase(it);
	else
		return;

	delete f;
}

void FMod::DeleteDirectory(const FString& path)
{
	FDirectory* dir = FindDirectory(path);
	if (!dir)
		return;

	std::filesystem::remove_all((Path() + "/" + dir->GetPath()).c_str());

	FDirectory* parent = dir->parent;
	auto it = parent->directories.Find(dir);
	if (it != parent->directories.end())
		parent->directories.Erase(it);
	else
		return;

	delete dir;
}

FFile* FMod::FindFile(const FString& path) const
{
	const char* pathPtr = path.c_str();

	const FDirectory* dir = &root;
	FString target;
	while (dir)
	{
		bool bIsFile = false;
		target.Clear();

		while (true)
		{
			if (pathPtr[0] == '\0')
			{
				bIsFile = true;
				break;
			}
			if (pathPtr[0] == '\\' || pathPtr[0] == '/')
			{
				pathPtr++;
				break;
			}
			target += pathPtr[0];
			pathPtr++;
		}

		if (!bIsFile)
		{
			FDirectory* newDir = nullptr;
			for (auto& d : dir->directories)
			{
				if (d->name == target)
				{
					newDir = d; 
					break;
				}
			}

			dir = newDir;
			continue;
		}
		
		for (auto f : dir->files)
			if (f->Name() + f->Extension() == target)
				return f;

		return nullptr;
	}
	return nullptr;
}

FFile* FMod::CreateFile(const FString& path)
{
	FString dirPath = path;
	if (dirPath[0] == '/' || dirPath[0] == '\\')
		dirPath.Erase(dirPath.begin());
	if (SizeType i = dirPath.FindLastOf("\\/"); i != -1)
		dirPath.Erase(dirPath.begin() + i, dirPath.end());
	else
		dirPath = FString();

	FDirectory* dir = nullptr;
	if (!dirPath.IsEmpty())
	{
		dir = FindDirectory(dirPath);
		if (!dir)
			dir = CreateDir(dirPath);
	}
	else
		dir = &root;

	FString fileName;
	FString fileExt;

	fileName = path;
	if (SizeType i = fileName.FindLastOf("\\/"); i != -1)
		fileName.Erase(fileName.begin(), fileName.begin() + i + 1);
	fileExt = fileName;

	if (FFile* f = dir->GetFile(fileName); f)
		return f;

	SizeType dotIndex = fileName.FindLastOf('.');
	if (dotIndex != -1)
	{
		fileName.Erase(fileName.begin() + dotIndex, fileName.end());
		fileExt.Erase(fileExt.begin(), fileExt.begin() + dotIndex);
	}

	FFile* file = new FFile();
	dir->files.Add(file);

	file->dir = dir;
	file->mod = this;
	file->name = fileName;
	file->extension = fileExt;

	//fs::create_directories((this->path + L"\\" + dirPath).c_str());

	FString _p = this->path + "/" + file->Path();
	CFStream stream(_p, "wb");
	if (!stream.IsOpen())
		CONSOLE_LogWarning("CFileSystem", FString("Failed to create OS file '") + _p + "'");

	stream.Close();
	return file;
}

bool FMod::MoveFile(FFile* file, const FString& destination)
{
	if (!file)
		return false;

	FDirectory* newDir = FindDirectory(destination);
	if (!newDir)
	{
		CONSOLE_LogWarning("CFileSystem", "Failed to move file, destination does not exist!");
		return false;
	}
	
	for (auto* f : newDir->files)
	{
		if (f->name == file->name && f->extension == file->extension)
		{
			CONSOLE_LogWarning("CFileSystem", "Failed to move file, file with the same name already exists in destination!");
			return false;
		}
	}

	FString oldPath = file->FullPath();

	FDirectory* oldDir = file->dir;
	oldDir->files.Erase(oldDir->files.Find(file));
	newDir->files.Add(file);

	file->dir = newDir;

	std::filesystem::rename(oldPath.c_str(), file->FullPath().c_str());
	CResourceManager::OnResourceFileMoved(file);
	return true;
}

bool FMod::MoveDirectory(FDirectory* dir, const FString& destination)
{
	if (!dir)
		return false;

	FDirectory* newDir = destination.IsEmpty() ? &root : FindDirectory(destination);
	for (auto* d : newDir->directories)
	{
		if (d->name == dir->name)
		{
			CONSOLE_LogWarning("CFileSystem", "Failed to move directory, directory with the same name already exsists in destination!");
			return false;
		}
	}

	FString oldPath = Path() + "/" + dir->GetPath();
	dir->parent->directories.Erase(dir->parent->directories.Find(dir));
	newDir->directories.Add(dir);

	dir->parent = newDir;

	std::filesystem::rename(oldPath.c_str(), (Path() + "/" + dir->GetPath()).c_str());

	dir->OnMoved();
	return true;
}

FFile* CFileSystem::FindFile(const FString& path)
{
	for (auto& m : Mods)
		if (FFile* f = m->FindFile(path); f != nullptr)
			return f;

	return nullptr;
}

FDirectory* CFileSystem::FindDirectory(const FString& path)
{
	for (auto& m : Mods)
		if (FDirectory* f = m->FindDirectory(path); f != nullptr)
			return f;

	return nullptr;
}

FMod* CFileSystem::FindMod(const FString& mod)
{
	for (auto& m : Mods)
		if (m->Name() == mod)
			return m;

	return nullptr;
}

void CFileSystem::MountDir(FMod* mod, const FString& path, FDirectory* dir)
{
	FString _path = path;
	if (*_path.last() == '\\' || *_path.last() == '/')
		_path.Erase(_path.last());

	for (auto& entry : fs::directory_iterator(_path.c_str()))
	{
		if (entry.is_directory())
		{
			auto dirName = entry.path().stem();
			//if (dirName == "bin")
			//	continue;
			//if (dirName == "config")
			//	continue;
			//if (dirName == "addons")
			//	continue;

			if (IsBlacklisted(dirName.generic_string().c_str()))
				continue;

			dir->directories.Add(new FDirectory());
			FDirectory* newDir = dir->directories.last();
			newDir->name = entry.path().stem().generic_string().c_str();
			newDir->parent = dir;
			MountDir(mod, _path + "/" + newDir->GetName(), newDir);
			continue;
		}

		if (!entry.is_regular_file())
			continue;

		if (entry.path().extension() == ".pak")
			continue; // TDOO: Load pak file.

		FFile* file = new FFile();
		file->mod = mod;

		file->name = entry.path().stem().generic_string().c_str();
		file->extension = entry.path().extension().generic_string().c_str();
		file->dir = dir;
		dir->files.Add(file);
	}
}

FMod* CFileSystem::MountMod(const FString& modPath, const FString& mn, const FString& sdkPath)
{
	// First check if the directory exists and then get its name.
	if (!FFileHelper::DirectoryExists(modPath))
		return nullptr;

	CONSOLE_LogInfo("CFileSystem", FString("Mounting Mod: ") + modPath);

	FString modName;
	if (mn.IsEmpty())
	{
		modName = modPath;
		SizeType slashI = modPath.FindLastOf("/\\");
		if (slashI != -1)
			modName.Erase(modName.begin(), modName.At(slashI + 1));
	}
	else
		modName = mn;

	Mods.Add(new FMod());

	FMod* mod = *Mods.last();
	mod->name = modName;
	mod->path = modPath;
	MountDir(mod, modPath, &mod->root);

#ifdef IS_DEV
	if (!sdkPath.IsEmpty())
	{
		if (FFileHelper::DirectoryExists(sdkPath))
			mod->sdkPath = sdkPath;
	}
	else if (modName != "Engine")
	{
		FString sdkPath = FString(".project/") + modName + "/sdk_content";
		if (FFileHelper::DirectoryExists(sdkPath))
		{
			//for (auto& entry : fs::recursive_directory_iterator(sdkPath.c_str()))
			//{
			//	// sdk_content should be a mirror of the content folder, so there's no reason to mount it here.
			//	if (entry.is_directory())
			//		continue;

			//	WString filePath = entry.path().c_str();
			//}

			mod->sdkPath = sdkPath;
		}
		//else
		//	CONSOLE_LogWarning(FString("Failed to load skd_content for mod '") + ToFString(modName) + "'!");
	}
#endif

	CResourceManager::ScanMod(mod);
	return mod;
}

bool CFileSystem::UnmountMod(FMod* mod)
{
	SizeType modIndex = 0;
	for (int i = 0; i < Mods.Size(); i++)
	{
		if (Mods[i] == mod)
		{
			modIndex = i;
			break;
		}
	}

	if (!modIndex)
		return false;

	CResourceManager::DeleteResourcesFromMod(mod);

	Mods.Erase(Mods.At(modIndex));
	delete mod;
	return true;
}

bool CFileSystem::IsBlacklisted(const FString& path)
{
	const char* blackListedPaths[] = {
		"config",
		"addons",
		"bin",
	};

	for (int i = 0; i < 3; i++)
	{
		if (path == blackListedPaths[i])
			return true;
	}
	return false;
}

#ifdef DEBUG
bool CFileSystem::ReloadMod(FMod* mod)
{
	SizeType modIndex = 0;
	for (int i = 0; i < Mods.Size(); i++)
	{
		if (Mods[i] == mod)
		{
			modIndex = i;
			break;
		}
	}

	if (!modIndex)
		return false;



	return true;
}
#endif

FDirectory::~FDirectory()
{
	for (auto f : files)
		delete f;
	for (auto d : directories)
		delete d;
}

FString FDirectory::GetPath() const
{
	if (!parent || parent->name.IsEmpty()) 
		return name; 
	return parent->GetPath() + "/" + name;
}

FFile* FDirectory::GetFile(const FString& name)
{
	for (auto* f : files)
	{
		if (f->Name() + f->Extension() == name)
			return f;
	}
	return nullptr;
}

void FDirectory::OnMoved()
{
	for (auto& d : directories)
		d->OnMoved();

	for (auto& f : files)
		CResourceManager::OnResourceFileMoved(f);
}

#if _WIN32
#include "windows.h"
#endif

void CFileSystem::SetCurrentPath(const FString& path)
{
#if _WIN32
	SetCurrentDirectoryA(path.c_str());
#else
	chdir(path.c_str());
#endif
}

FString CFileSystem::GetCurrentPath()
{
	char buff[128];
#if _WIN32
	GetCurrentDirectoryA(128, buff);
#else
	getcwd(buff, 128);
#endif
	return buff;
}

FFile::~FFile()
{
	CResourceManager::OnResourceFileDeleted(this);
}

bool FFile::SetName(const FString& n)
{
	for (auto* f : dir->files)
		if (f->name == n && f->extension == extension)
			return false;

	FString oldPath = FullPath();
	name = n;

	std::filesystem::rename(oldPath.c_str(), FullPath().c_str());
	return true;
}

bool FFile::SetExtension(const FString& e)
{
	for (auto* f : dir->files)
		if (f->name == name && f->extension == e)
			return false;

	FString oldPath = FullPath();
	extension = e;

	std::filesystem::rename(oldPath.c_str(), FullPath().c_str());
	return true;
}

CFStream FFile::GetSdkStream(const char* mode)
{
	FString sdkPath = GetSdkPath();
	if (sdkPath.IsEmpty())
		return CFStream();
	return CFStream(sdkPath, mode);
}

void CFileSystem::OSCreateDirectory(const FString& path)
{
	std::filesystem::create_directories(path.c_str());
}
