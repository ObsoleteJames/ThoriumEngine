
#include "System.h"

#include <windows.h>
#include <shlobj.h>

int SSystem::Execute(const char* cmd, bool bDetached)
{
	PROCESS_INFORMATION ht{};
	STARTUPINFO si{};
	si.cb = sizeof(si);
	int r = CreateProcessA(NULL, (char*)cmd, nullptr, nullptr, false, 0, nullptr, nullptr, &si, &ht);
	if (r != 0)
		return r;

	if (!bDetached)
	{
		WaitForSingleObject(ht.hProcess, INFINITE);

		DWORD ec;
		GetExitCodeProcess(ht.hProcess, &ec);
		r = ec;

		CloseHandle(ht.hProcess);
		CloseHandle(ht.hThread);
	}
	return r;
}

FString SSystem::GetEnginePath(const char* version)
{
	FString keyPath = FString("SOFTWARE\\ThoriumEngine\\") + version;

	HKEY hKey;
	LONG lRes = RegOpenKeyEx(HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_READ, &hKey);
	if (lRes == ERROR_FILE_NOT_FOUND)
		return "";

	CHAR strBuff[MAX_PATH];
	DWORD buffSize = sizeof(strBuff);
	lRes = RegQueryValueEx(hKey, "path", 0, NULL, (LPBYTE)strBuff, &buffSize);
	if (lRes != ERROR_SUCCESS)
		return "";

	return FString(strBuff);
}

FString SSystem::GetDataPath()
{
	PWSTR appdata;
	if (FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &appdata)))
		return FString();

	char r[MAX_PATH];
	wcstombs(r, appdata, MAX_PATH);

	return FString(r);
}

FString SSystem::GetDocumentsPath()
{
	PWSTR appdata;
	if (FAILED(SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &appdata)))
		return FString();

	char r[MAX_PATH];
	wcstombs(r, appdata, MAX_PATH);

	return FString(r);
}

FString SSystem::OpenFileDialog(const char* filter)
{
	OPENFILENAMEA ofn;
	CHAR szFile[255] = { 0 };
	CHAR currentDir[255] = { 0 };
	ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);

	if (GetCurrentDirectoryA(255, currentDir))
		ofn.lpstrInitialDir = currentDir;

	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameA(&ofn) == TRUE)
		return ofn.lpstrFile;

	return FString();
}

TArray<FString> SSystem::OpenFilesDialog(const char* filter)
{
	OPENFILENAMEA ofn;
	CHAR szFile[512] = { 0 };
	CHAR currentDir[512] = { 0 };
	ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);

	if (GetCurrentDirectoryA(255, currentDir))
		ofn.lpstrInitialDir = currentDir;

	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT;

	if (GetOpenFileNameA(&ofn) == TRUE)
	{
		TArray<FString> r;
		FString path = ofn.lpstrFile;
		if (ofn.nFileOffset > 0)
		{
			FString dir = path;
			dir.Erase(dir.begin() + (ofn.nFileOffset - 1), dir.end());
			path.Erase(path.begin(), path.begin() + ofn.nFileOffset);

			r = path.Split(' ');

			for (int i = 0; i < r.Size(); i++)
			{
				r[i] = dir + r[i];
			}
		}
		else
			r.Add(path);

		return r;
	}

	return TArray<FString>();
}

FString SSystem::SaveFileDialog(const char* filter)
{
	OPENFILENAMEA ofn;
	CHAR szFile[256] = { 0 };
	CHAR currentDir[256] = { 0 };
	ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);

	if (GetCurrentDirectoryA(256, currentDir))
		ofn.lpstrInitialDir = currentDir;

	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	ofn.lpstrDefExt = strchr(filter, '\0') + 1;

	if (GetSaveFileNameA(&ofn) == TRUE)
		return ofn.lpstrFile;

	return FString();
}

FString SSystem::OpenFolderDialog()
{
	BROWSEINFO bi = { 0 };
	bi.ulFlags = BIF_USENEWUI;
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

	CHAR currentDir[256] = { 0 };
	if (GetCurrentDirectoryA(256, currentDir))
		bi.lParam = (LPARAM)currentDir;

	if (pidl != NULL)
	{
		TCHAR path[MAX_PATH];
		if (SHGetPathFromIDList(pidl, path))
		{
			FString sPath = path;
			return sPath;
		}
	}

	return FString();
}

void SSystem::OpenFileManager(const char* path)
{
	ShellExecuteA(NULL, "open", path, NULL, NULL, SW_SHOWDEFAULT);
}

void SSystem::OpenFile(const char* path)
{
	ShellExecuteA(NULL, "open", path, NULL, NULL, SW_SHOWDEFAULT);
}

ENGINE_API void SSystem::SetClipboardData(const FString& txt)
{
	if (!OpenClipboard(NULL))
		return;
	EmptyClipboard();

	HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (txt.Size() + 1));
	if (!hglbCopy)
	{
		CloseClipboard();
		return;
	}

	LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hglbCopy);
	memcpy(lptstrCopy, txt.Data(), txt.Size());
	lptstrCopy[txt.Size()] = (TCHAR)0;    // null character 
	GlobalUnlock(hglbCopy);

	::SetClipboardData(CF_TEXT, hglbCopy);

	CloseClipboard();
}

ENGINE_API FString SSystem::GetClipboardData()
{
	return FString();
}
