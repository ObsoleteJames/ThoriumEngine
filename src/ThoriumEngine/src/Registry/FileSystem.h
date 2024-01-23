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

	inline const FString& GetName() const { return name; }
	FString GetPath() const { if (!parent || parent->name.IsEmpty()) return name; return parent->GetPath() + "/" + name; }

	inline const TArray<FDirectory*>& GetSubDirectories() const { return directories; }
	FFile* GetFile(const FString& file);

	inline const TArray<FFile*>& GetFiles() const { return files; }

	inline FDirectory* Parent() const { return parent; }

private:
	FString name;
	FDirectory* parent = nullptr;
	TArray<FFile*> files;
	TArray<FDirectory*> directories;
};

struct ENGINE_API FMod
{
	friend class CFileSystem;
	friend class CResourceManager;

public:
	inline const FString& Name() const { return name; }
	inline const FString& Path() const { return path; }

	FDirectory* FindDirectory(const FString& path) const;
	FDirectory* CreateDir(const FString& path);

	void DeleteFile(const FString& path);
	void DeleteDirectory(const FString& path);

	FFile* FindFile(const FString& path) const;
	FFile* CreateFile(const FString& path);

	inline FDirectory* GetRootDir() { return &root; }

	inline const FString& GetSdkPath() const { return sdkPath; }
	inline void SetSdkPath(const FString& path) { sdkPath = path; }
	inline bool HasSdkContent() const { return !sdkPath.IsEmpty(); }

private:
	FString name;
	FString path; // Path to the mods content
	FString sdkPath; // Path to the sdk_content folder

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

	inline FString Path() const { return (!dir->name.IsEmpty()) ? (dir->GetPath() + "/" + name + extension) : (name + extension); }
	inline FString FullPath() const { return mod->Path() + "/" + Path(); }
	inline const FString& Name() const { return name; }
	inline const FString& Extension() const { return extension; }
	inline SizeType Size() const { return size; }
	inline FMod* Mod() const { return mod; }
	inline FDirectory* Dir() const { return dir; }

	inline IBaseFStream* GetStream(const char* mode)
	{
		if (pak)
			return nullptr; // TODO: return pak stream.

		return new CFStream(mod->Path() + "/" + Path(), mode);
	}

	CFStream GetSdkStream(const char* mode);

	inline FString GetSdkPath() const { if (mod->HasSdkContent()) { return mod->GetSdkPath() + "/" + Path() + ".meta"; } return FString(); }

private:
	FString name;
	FString extension;
	SizeType size;

	FDirectory* dir;
	FMod* mod;
	CPak* pak = nullptr;
	void* pakFileEntry = nullptr;
};

class ENGINE_API CFileSystem
{
public:
	static FFile* FindFile(const FString& path);
	static FDirectory* FindDirectory(const FString& path);

	static FMod* FindMod(const FString& mod);

	static FMod* MountMod(const FString& modPath, const FString& modName = FString(), const FString& sdkPath = FString());
	static bool UnmountMod(FMod* mod);

	static inline const TArray<FMod*>& GetMods() { return Mods; }

	static void SetCurrentPath(const FString& path);
	static FString GetCurrentPath();

	static void OSCreateDirectory(const FString& path);

#if IS_DEV
	static bool ReloadMod(FMod* mod);
#endif

private:
	static void MountDir(FMod* mod, const FString& path, FDirectory* dir);

private:
	//TArray<FFile> Files;
	static TArray<FMod*> Mods;
};
