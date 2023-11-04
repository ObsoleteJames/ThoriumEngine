#pragma once

#include "EngineCore.h"

struct FFile;
struct FMod;
struct FDirectory;
class FAssetClass;

class CAssetBrowserWidget
{
public:
	// Must be called inside of a window or child window.
	void RenderUI(float width = 0.f, float height = 0.f);

	inline FFile* GetSelectedFile() const { if (selectedFiles.Size() == 0) return nullptr; return selectedFiles[0]; }
	inline const TArray<FFile*>& GetSelectedFiles() const { return selectedFiles; }

	void SetDir(FMod* m, FDirectory* d);
	void SetDir(const WString& m, const WString& d);

	// Operators
	void Back();
	void Forward();
	void Root();

	// Extract the mod and dir, format: 'MOD:\DIR\DIR2'
	bool ExtractPath(const WString& combined, WString& outMod, WString& outDir);

private:
	void DrawDirTree(FDirectory* dir, FDirectory* parent, FMod* mod);

	void SetSelectedFile(FFile* file);
	void AddSelectedFile(FFile* file);
	void RemoveSelectedFile(FFile* file);
	bool IsFileSelected(FFile* file);

public:
	bool bAllowFileEdit = true;
	bool bAllowMultiSelect = true;

	// true if the selected file was double clicked
	bool bDoubleClickedFile = false;

	// if set, only assets of this type will be visible
	FAssetClass* viewFilter = nullptr;

	WString dir;
	WString mod = L"Engine";

	int iconsSize = 2;

private:
	FString dirInput;

	SizeType historyIndex = 0;
	TArray<WString> historyList;

	TArray<FFile*> selectedFiles;

	float sizeL = 200;
	float sizeR = 200;

};
