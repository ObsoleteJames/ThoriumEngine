
#include "DataAsset.h"
#include "Console.h"

#define THDA_MAGIC_SIZE 27
static const char* thdaMagicStr = "\0\0ThoriumEngine Data Asset\0";

void CDataAsset::Init()
{
	if (!file)
		return;

	Load(0);
}

void CDataAsset::Save()
{
	TUniquePtr<IBaseFStream> stream = file->GetStream("wb");
	if (!stream || !stream->IsOpen())
	{
		CONSOLE_LogError("CDataAsset", FString("Failed to create file stream for '") + ToFString(file->Path()) + "'");
		return;
	}

	stream->Write((void*)thdaMagicStr, THDA_MAGIC_SIZE);

	FMemStream data;
	for (FClass* c = GetClass(); c != CDataAsset::StaticClass(); c = c->GetBaseClass())
		SerializeProperties(data, c, this);

	SizeType size = data.Size();
	*stream << &size;

	stream->Write(data.Data(), data.Size());
}

void CDataAsset::Load(uint8)
{
	TUniquePtr<IBaseFStream> stream = file->GetStream("rb");
	if (!stream || !stream->IsOpen())
	{
		CONSOLE_LogError("CDataAsset", FString("Failed to create file stream for '") + ToFString(file->Path()) + "'");
		return;
	}

	char magicStr[THDA_MAGIC_SIZE];
	stream->Read(magicStr, THDA_MAGIC_SIZE);

	if (memcmp(thdaMagicStr, magicStr, THDA_MAGIC_SIZE) != 0)
	{
		CONSOLE_LogError("CDataAsset", FString("Invalid file type '") + ToFString(file->Path()) + "'");
		return;
	}

	SizeType dataSize = 0;
	*stream >> &dataSize;

	FMemStream data;
	data.Resize(dataSize);

	stream->Read(data.Data(), dataSize);

	for (FClass* c = GetClass(); c != CDataAsset::StaticClass(); c = c->GetBaseClass())
		LoadProperties(data, c, this);
}

void CDataAsset::Unload(uint8)
{
}

bool CDataAsset::Import(const WString&)
{
	return false;
}
