
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

	TMap<SizeType, TObjectPtr<CEntity>>& ents = world->entities;
	SizeType numEntsOffset = stream->Tell();
	SizeType numEnts = 0;
	*stream << &numEnts;

	for (auto& ent : ents)
	{
		SizeType entId = ent.second->EntityId();
		SizeType dataSize;

#if INCLUDE_EDITOR_DATA
		if (ent.second->bEditorEntity)
			continue;
#endif

		FMemStream data;
		ent.second->Serialize(data);

		dataSize = data.Size();
		*stream << ent.second->GetClass()->GetInternalName() << &entId << &dataSize;

		stream->Write(data.Data(), data.Size());
		numEnts++;
	}

	stream->Seek(numEntsOffset, SEEK_SET);
	*stream << &numEnts;
}
