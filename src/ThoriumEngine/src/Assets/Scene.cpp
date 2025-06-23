
#include "Scene.h"
#include "Console.h"
#include "Game/World.h"
#include "Game/Entity.h"
#include <Util/FStream.h>

void CScene::OnSave(IBaseFStream* stream)
{
	uint sig = CSCENE_SIGNITURE;

	*stream << &sig;

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
		SizeType numComps = ent.second->GetAllComponents().size();

#if INCLUDE_EDITOR_DATA
		if (ent.second->bEditorEntity)
			continue;
#endif

		*stream << ent.second->GetClass()->GetInternalName() << &entId << &numComps;

		for (auto& comp : ent.second->GetAllComponents())
		{
			*stream << comp.second->GetClass()->GetInternalName();
			*stream << comp.second->Name();

			SizeType id = comp.first;
			*stream << &id;

			bool bUserCreated = comp.second->IsUserCreated();
			*stream << &bUserCreated;
		}
	}

	for (auto& ent : ents)
	{
		SizeType entId = ent.second->EntityId();
		SizeType dataSize;

		FMemStream data;
		ent.second->Serialize(data);

		dataSize = data.Size();
		*stream << &entId << &dataSize;

		stream->Write(data.Data(), data.Size());
		numEnts++;
	}

	stream->Seek(numEntsOffset, SEEK_SET);
	*stream << &numEnts;
}

uint8 CScene::GetFileVersion() const
{
	return CSCENE_VERSION;
}
