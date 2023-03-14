
#include <string>
#include "FileSystem.h"
#include "Engine.h"
#include "Misc/FileHelper.h"
#include "Resources/ResourceManager.h"
#include "Console.h"
#include <filesystem>

TArray<FMod*> CFileSystem::Mods;
namespace fs = std::filesystem;

FDirectory* FMod::FindDirectory(const WString& path) const
{
	const wchar_t* pathPtr = path.c_str();

	const FDirectory* dir = &root;
	WString target;
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

FDirectory* FMod::CreateDir(const WString& path)
{
	const wchar_t* pathPtr = path.c_str();

	if (pathPtr[0] == L'\\' || pathPtr[0] == L'/')
		pathPtr++;

	FDirectory* dir = &root;
	WString target;
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

	fs::create_directories((this->path + L"\\" + path).c_str());
	if (gIsEditor && HasSdkContent())
		fs::create_directories((sdkPath + L"\\" + path).c_str());
	return dir;
}

void FMod::DeleteFile(const WString& path)
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

void FMod::DeleteDirectory(const WString& path)
{
	FDirectory* dir = FindDirectory(path);
	if (!dir)
		return;

	std::filesystem::remove_all((Path() + L"\\" + dir->GetPath()).c_str());

	FDirectory* parent = dir->parent;
	auto it = parent->directories.Find(dir);
	if (it != parent->directories.end())
		parent->directories.Erase(it);
	else
		return;

	delete dir;
}

FFile* FMod::FindFile(const WString& path) const
{
	const wchar_t* pathPtr = path.c_str();

	const FDirectory* dir = &root;
	WString target;
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

FFile* FMod::CreateFile(const WString& path)
{
	WString dirPath = path;
	if (dirPath[0] == L'/' || dirPath[0] == L'\\')
		dirPath.Erase(dirPath.begin());
	if (SizeType i = dirPath.FindLastOf(L"\\/"); i != -1)
		dirPath.Erase(dirPath.begin() + i, dirPath.end());
	else
		dirPath = L"";

	FDirectory* dir = nullptr;
	if (!dirPath.IsEmpty())
	{
		dir = FindDirectory(dirPath);
		if (!dir)
			dir = CreateDir(dirPath);
	}
	else
		dir = &root;

	WString fileName;
	WString fileExt;

	fileName = path;
	if (SizeType i = fileName.FindLastOf(L"\\/"); i != -1)
		fileName.Erase(fileName.begin(), fileName.begin() + i + 1);
	fileExt = fileName;

	if (FFile* f = dir->GetFile(fileName); f)
		return f;

	SizeType dotIndex = fileName.FindLastOf(L'.');
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

	FString _p = ToFString(this->path + L"\\" + file->Path());
	CFStream stream(_p, "wb");
	if (!stream.IsOpen())
		CONSOLE_LogWarning(FString("Failed to create OS file '") + _p + "'");

	stream.Close();
	return file;
}

FFile* CFileSystem::FindFile(const WString& path)
{
	for (auto& m : Mods)
		if (FFile* f = m->FindFile(path); f != nullptr)
			return f;

	return nullptr;
}

FDirectory* CFileSystem::FindDirectory(const WString& path)
{
	for (auto& m : Mods)
		if (FDirectory* f = m->FindDirectory(path); f != nullptr)
			return f;

	return nullptr;
}

FMod* CFileSystem::FindMod(const WString& mod)
{
	for (auto& m : Mods)
		if (m->Name() == mod)
			return m;

	return nullptr;
}

void CFileSystem::MountDir(FMod* mod, const WString& path, FDirectory* dir)
{
	WString _path = path;
	if (*_path.last() == L'\\' || *_path.last() == L'/')
		_path.Erase(_path.last());

	for (auto& entry : fs::directory_iterator(_path.c_str()))
	{
		if (entry.is_directory())
		{
			auto dirName = entry.path().stem();
			if (dirName == L"bin")
				continue;
			if (dirName == L"config")
				continue;
			if (dirName == L"addons")
				continue;

			dir->directories.Add(new FDirectory());
			FDirectory* newDir = dir->directories.last();
			newDir->name = entry.path().stem().c_str();
			newDir->parent = dir;
			MountDir(mod, _path + L"\\" + newDir->GetName(), newDir);
			continue;
		}

		if (!entry.is_regular_file())
			continue;

		if (entry.path().extension() == ".pak")
			continue; // TDOO: Load pak file.

		FFile* file = new FFile();
		file->mod = mod;

		file->name = entry.path().stem().c_str();
		file->extension = entry.path().extension().c_str();
		file->dir = dir;
		dir->files.Add(file);
	}
}

FMod* CFileSystem::MountMod(const WString& modPath, const WString& mn, const WString& sdkPath)
{
	// First check if the directory exists and then get its name.
	if (!FFileHelper::DirectoryExists(modPath))
		return nullptr;

	CONSOLE_LogInfo(FString("Mounting Mod: ") + ToFString(modPath));

	WString modName;
	if (mn.IsEmpty())
	{
		modName = modPath;
		SizeType slashI = modPath.FindLastOf(L"/\\");
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
	else if (modName != L"Engine")
	{
		WString sdkPath = WString(L".project\\") + modName + L"\\sdk_content";
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

	Mods.Erase(Mods.At(modIndex));
	return true;
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

FFile* FDirectory::GetFile(const WString& name)
{
	for (auto* f : files)
	{
		if (f->Name() + f->Extension() == name)
			return f;
	}
	return nullptr;
}

#include "windows.h"

void CFileSystem::SetCurrentPath(const WString& path)
{
#if _WIN32
	SetCurrentDirectoryW(path.c_str());
#endif
}

FFile::~FFile()
{
	CResourceManager::OnResourceFileDeleted(this);
}
