#pragma once

#include "EngineCore.h"

class FAssetClass;
struct FFile;

namespace ThoriumEditor
{
	// returns false if a popup is already open
	// Once OpenFile/SaveFile are called, AcceptFile() has to be called in order to close the popup.
	bool OpenFile(const FString& id, FAssetClass* type, const WString& dir = WString());
	bool SaveFile(const FString& id, FAssetClass* type, const WString& dir = WString());

	// Returns true if a file dialog with the corresponding id is open
	bool AcceptFile(const FString& id, WString* outFile, WString* outMod = nullptr);
	
	void Cancel(const FString& id);
}
