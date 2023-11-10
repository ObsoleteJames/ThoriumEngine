#pragma once

#include "EngineCore.h"
#include <Util/FStream.h>

#define CPak void

struct FDirectory;
struct FFile;

struct ENGINE_API FDirectory
{
	friend struct FMod;
	friend struct FFile;
	friend class CFileSystem;
	friend class CResourceManager;

public:
	~FDirectory();

	inline const WString& GetName() const { return name; }
	WString GetPath() const { if (!parent || parent->name.IsEmpty()) return name; return parent->GetPath() + L"\\" + name; }

	inline const TArray<FDirectory*>& GetSubDirectories() const { return directories; }
	FFile* GetFile(const WString& file);

	inline const TArray<FFile*>& GetFiles() const { return files; }

	inline FDirectory* Parent() const { return parent; }

private:
	WString name;
	FDirectory* parent = nullptr;
	TArray<FFile*> files;
	TArray<FDirectory*> directories;
};

struct ENGINE_API FMod
{
	friend class CFileSystem;
	friend class CResourceManager;

public:
	inline const WString& Name() const { return name; }
	inline const WString& Path() const { return path; }

	FDirectory* FindDirectory(const WString& path) const;
	FDirectory* CreateDir(const WString& path);

	void DeleteFile(const WString& path);
	void DeleteDirectory(const WString& path);

	FFile* FindFile(const WString& path) const;
	FFile* CreateFile(const WString& path);

	inline FDirectory* GetRootDir() { return &root; }

	inline const WString& GetSdkPath() const { return sdkPath; }
	inline void SetSdkPath(const WString& path) { sdkPath = path; }
	inline bool HasSdkContent() const { return !sdkPath.IsEmpty(); }

private:
	WString name;
	WString path; // Path to the mods content
	WString sdkPath; // Path to the sdk_content folder

	SizeType numFiles;

	FDirectory root;
	TArray<CPak*> Paks;
};

struct ENGINE_API FFile
{
	friend class CFileSystem;
	friend struct FMod;

public:
	~FFile();

	inline WString Path() const { return (!dir->name.IsEmpty()) ? (dir->GetPath() + L"\\" + name + extension) : (name + extension); }
	inline WString FullPath() const { return mod->Path() + L"\\" + Path(); }
	inline const WString& Name() const { return name; }
	inline const WString& Extension() const { return extension; }
	inline SizeType Size() const { return size; }
	inline FMod* Mod() const { return mod; }
	inline FDirectory* Dir() const { return dir; }

	inline IBaseFStream* GetStream(const char* mode)
	{
		if (pak)
			return nullptr; // TODO: return pak stream.

		return new CFStream(mod->Path() + L"\\" + Path(), ToWString(mode).c_str());
	}

	CFStream GetSdkStream(const char* mode);

	inline WString GetSdkPath() const { if (mod->HasSdkContent()) { return mod->GetSdkPath() + L"\\" + Path() + L".meta"; } return WString(); }

private:
	WString name;
	WString extension;
	SizeType size;

	FDirectory* dir;
	FMod* mod;
	CPak* pak = nullptr;
	void* pakFileEntry = nullptr;
};

class ENGINE_API CFileSystem
{
public:
	static FFile* FindFile(const WString& path);
	static FDirectory* FindDirectory(const WString& path);

	static FMod* FindMod(const WString& mod);

	static FMod* MountMod(const WString& modPath, const WString& modName = WString(), const WString& sdkPath = WString());
	static bool UnmountMod(FMod* mod);

	static inline const TArray<FMod*>& GetMods() { return Mods; }

	static void SetCurrentPath(const WString& path);
	static WString GetCurrentPath();

#if IS_DEV
	static bool ReloadMod(FMod* mod);
#endif

private:
	static void MountDir(FMod* mod, const WString& path, FDirectory* dir);

private:
	//TArray<FFile> Files;
	static TArray<FMod*> Mods;
};
