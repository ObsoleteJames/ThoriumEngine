
#include <string>
#include "Engine.h"
#include "CodeGenerator.h"
#include "Registry/FileSystem.h"
#include <Util/Map.h>

static TMap<std::string, FString> classIncludes = {
	{ "CEntity", "Game/Entity.h" },
	{ "CPawn", "Game/Pawn.h" },
	{ "CPlayerController", "Game/PlayerController.h" },
	{ "CGameMode", "Game/GameMode.h" },
	{ "CGameInstance", "Game/GameInstance.h" },
	{ "CEntityComponent", "Game/EntityComponent.h" },
	{ "CSceneComponent", "Game/Components/SceneComponent.h" },
};

void CCodeGenerator::GenerateCppFile(const FString& filePath, const FString& baseClass, const WString& modName /*= FString()*/)
{
	FMod* mod = modName.IsEmpty() ? gEngine->ActiveGame().mod : CFileSystem::FindMod(modName);
	if (!mod)
		return;

	FString className = filePath;
	if (auto i = className.FindLastOf("\\/"); i != -1)
		className.Erase(className.begin(), className.begin() + i + 1);
	if (auto i = className.FindLastOf('.'); i != -1)
		className.Erase(className.begin() + i, className.end());

	FString classHeader = className;
	className = "C" + className;

	TMap<std::string, FString> baseVariables;
	baseVariables["CLASS_NAME"] = className;
	baseVariables["CLASS_HEADER"] = classHeader;
	baseVariables["BASE_CLASS"] = baseClass;
	baseVariables["BASE_CLASS_INCLUDE"] = GetIncludePath(baseClass);

	WString srcPath = mod->GetSdkPath() + L"\\..\\src";

	// Write the header file.
	{
		FString headerFile = ParseBaseFile(L"editor\\CppBaseFiles\\Class.h", baseVariables);

		CFStream stream(srcPath + L"\\" + ToWString(filePath) + L".h", L"wb");
		if (stream.IsOpen())
		{
			stream.Write(headerFile.Data(), headerFile.Size());
		}
	}

	// Write the source file.
	{
		FString sourceFile = ParseBaseFile(L"editor\\CppBaseFiles\\Class.cpp", baseVariables);

		CFStream stream(srcPath + L"\\" + ToWString(filePath) + L".cpp", L"wb");
		if (stream.IsOpen())
		{
			stream.Write(sourceFile.Data(), sourceFile.Size());
		}
	}
}

FString CCodeGenerator::GetIncludePath(const FString& className)
{
	auto it = classIncludes.find(className.c_str());
	if (it != classIncludes.end())
		return it->second;
	return FString();
}

FString CCodeGenerator::ParseBaseFile(const WString& file, const TMap<std::string, FString>& variables)
{
	auto* f = CFileSystem::FindFile(file);
	if (!f)
		return FString();

	SizeType fileSize = 0;
	FString r;

	// Read the file's content
	{
		TUniquePtr<IBaseFStream> stream = f->GetStream("rb");
		stream->Seek(0, SEEK_END);
		fileSize = stream->Tell();
		stream->Seek(0, SEEK_SET);

		char* data = (char*)malloc(fileSize + 1);
		stream->Read(data, fileSize);
		data[fileSize] = '\0';

		r = data;

		free(data);
	}

	for (SizeType i = r.Find("$("); i != -1; i = r.Find("$("))
	{
		FString var;
		SizeType start = i;

		i += 2;

		while (r[i] != ')')
			var += r[i++];

		r.Erase(r.begin() + start, r.begin() + i + 1);

		auto it = variables.find(var.c_str());
		if (it == variables.end())
			continue;

		FString value = it->second;

		for (SizeType y = value.Size(); y > 0; y--)
			r.Insert(value[y - 1], start);
	}
	return r;
}
