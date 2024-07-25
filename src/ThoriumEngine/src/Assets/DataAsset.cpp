
#include "DataAsset.h"
#include "Console.h"

void CDataAsset::OnInit(IBaseFStream* stream)
{
	if (!file)
		return;

	Load(0);
}

void CDataAsset::OnSave(IBaseFStream* stream)
{
	FMemStream data;
	for (FClass* c = GetClass(); c != CDataAsset::StaticClass(); c = c->GetBaseClass())
		SerializeProperties(data, c, this);

	SizeType size = data.Size();
	*stream << &size;

	stream->Write(data.Data(), data.Size());
}

void CDataAsset::OnLoad(IBaseFStream* stream, uint8)
{
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
