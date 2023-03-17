
#include "Entity.h"
#include "EntityComponent.h"
#include "Module.h"
#include "Console.h"
#include "Engine.h"
#include "Components/SceneComponent.h"

CEntityComponent* CEntity::AddComponent(FClass* type, const FString& name)
{
	CEntityComponent* comp = (CEntityComponent*)CreateObject(type, name);
	if (!comp)
		return nullptr;

	comp->ent = this;
	comp->SetOwner(this);
	comp->Init();

	components.Add(comp);
	if (world->IsActive())
		comp->OnStart();

	return comp;
}

CEntityComponent* CEntity::GetComponent(FClass* type, const FString& name)
{
	if (!name.IsEmpty())
	{
		for (auto& comp : components)
		{
			if (comp->GetClass() == type && comp->Name() == name)
				return comp;
		}
	}
	else
	{
		for (auto& comp : components)
		{
			if (comp->GetClass() == type)
				return comp;
		}
	}

	return nullptr;
}

void CEntity::RemoveComponent(CEntityComponent* comp)
{
	for (auto it = components.begin(); it != components.end(); it++)
	{
		if (*it == comp)
		{
			components.Erase(it);
			comp->Delete();
			break;
		}
	}
}

void CEntity::Init()
{
	rootComponent = AddComponent<CSceneComponent>("root");
}

void CEntity::OnStart()
{
	for (auto& comp : components)
		comp->OnStart();
}

void CEntity::OnStop()
{
}

void CEntity::Update(double dt)
{
	for (auto& comp : components)
		comp->Update(dt);
}

void CEntity::Serialize(FMemStream& out)
{
	BaseClass::Serialize(out);

	SizeType numComponents = components.Size();
	out << &numComponents;

	for (auto& comp : components)
	{
		// Write component class typename
		out << comp->GetClass()->GetInternalName();

		// Write the components name. this already gets written by the default serializer
		// but we need it earlier in order to write to the correct component when loading.
		out << comp->Name();

		FMemStream compOut;
		comp->Serialize(compOut);

		SizeType id = comp->Id();
		out << &id;

		bool bUserCreated = comp->IsUserCreated();
		out << &bUserCreated;

		SizeType compDataSize = compOut.Size();
		out << &compDataSize;

		out.Write(compOut.Data(), compOut.Size());
	}
}

void CEntity::Load(FMemStream& in)
{
	BaseClass::Load(in);

	SizeType numComponents;
	in >> &numComponents;

	TArray<TPair<CEntityComponent*, FMemStream>> comps;
	comps.Resize(numComponents);

	for (SizeType i = 0; i < numComponents; i++)
	{
		FString classTypename;
		in >> classTypename;

		FString compName;
		in >> compName;

		SizeType compId;
		in >> &compId;

		bool bUserCreated = false;
		in >> &bUserCreated;

		SizeType compDataSize;
		in >> &compDataSize;

		FClass* compClass = CModuleManager::FindClass(classTypename);
		if (!compClass)
		{
			CONSOLE_LogWarning("Serialized component has unkown class '" + classTypename + "', loading for entity " + Name() + ".");
			in.Seek(compDataSize, SEEK_CUR);
			continue;
		}

		CEntityComponent* comp = nullptr;
		if (!bUserCreated)
			comp = GetComponent(compClass, compName);
		if (!comp)
			comp = AddComponent(compClass, compName);

		comp->SetId(compId);
		comps[i].Key = comp;
		comps[i].Value.Resize(compDataSize);
		in.Read(comps[i].Value.Data(), compDataSize);
	}

	for (SizeType i = 0; i < numComponents; i++)
	{
		comps[i].Key->Load(comps[i].Value);
		if (comps[i].Key->bEditorOnly && !gIsEditor)
			RemoveComponent(comps[i].Key);
	}
}

void CEntity::OnDelete()
{
	for (auto& comp : components)
		comp->Delete();

	FWorldRegisterer::UnregisterEntity(world, this);
}

void CEntity::FireOutput(const FString& output)
{

}
