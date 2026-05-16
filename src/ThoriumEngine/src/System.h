#pragma once

#include <Util/Core.h>
#include "EngineCore.h"

namespace SSystem
{
	ENGINE_API int Execute(const char* cmd, bool bDetached = false);
	inline int Execute(const FString& cmd, bool bDetached = false) { return Execute(cmd.c_str(), bDetached); }

	/// ------ Paths ------

	ENGINE_API FString GetEnginePath(const char* version = ENGINE_VERSION);

	ENGINE_API FString GetDataPath();

	ENGINE_API FString GetDocumentsPath();

	ENGINE_API FString GetPlatformName();

	/// ------ Files ------

	ENGINE_API FString OpenFileDialog(const char* filter);
	inline FString OpenFileDialog(const FString& filter = FString()) { return OpenFileDialog(filter.c_str()); }

	ENGINE_API TArray<FString> OpenFilesDialog(const char* filter);
	inline TArray<FString> OpenFilesDialog(const FString& filter = FString()) { return OpenFilesDialog(filter.c_str()); }

	ENGINE_API FString SaveFileDialog(const char* filter);
	inline FString SaveFileDialog(const FString& filter = FString()) { return SaveFileDialog(filter.c_str()); }

	ENGINE_API FString OpenFolderDialog();

	// Open the folder/file in the systems file manager.
	ENGINE_API void OpenFileManager(const char* path);
	inline void OpenFileManager(const FString& path = FString()) { return OpenFileManager(path.c_str()); }

	// Open the file in the associated program for the file type.
	ENGINE_API void OpenFile(const char* path);
	inline void OpenFile(const FString& path = FString()) { return OpenFile(path.c_str()); }

	/// ------ Clipboard ------

	ENGINE_API void SetClipboardData(const FString& txt = FString());
	ENGINE_API FString GetClipboardData();
}
