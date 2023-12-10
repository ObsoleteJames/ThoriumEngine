
#include "Scene.h"
#include "Console.h"
#include "Game/World.h"
#include "Game/Entity.h"
#include <Util/FStream.h>

void CScene::Save(CWorld* world)
{
	TUniquePtr<IBaseFStream> stream = file->GetStream("wb");
	if (!stream || !stream->IsOpen())
	{
		CONSOLE_LogError("CWorld", FString("Failed to create file stream for '") + ToFString(file->Path()) + "'");
		return;
	}

	uint sig = CSCENE_SIGNITURE;
	uint8 version = CSCENE_VERSION;

	*stream << &sig << &version;

	if (gamemodeClass.Get())
	{
		*stream << gamemodeClass.Get()->GetInternalName();
	}
	else
		*stream << FString("");

	TArray<TObjectPtr<CEntity>>& ents = world->entities;
	SizeType numEntsOffset = stream->Tell();
	SizeType numEnts = 0;
	*stream << &numEnts;

	for (auto& ent : ents)
	{
		SizeType entId = ent->Id();
		SizeType dataSize;

#if INCLUDE_EDITOR_DATA
		if (ent->bEditorEntity)
			continue;
#endif

		FMemStream data;
		ent->Serialize(data);

		dataSize = data.Size();
		*stream << ent->GetClass()->GetInternalName() << &entId << &dataSize;

		stream->Write(data.Data(), data.Size());
		numEnts++;
	}

	stream->Seek(numEntsOffset, SEEK_SET);
	*stream << &numEnts;
}
