
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

	auto findC = components.find(comp->ComponentId());
	while (findC != components.end())
	{
		// re-generate the id
		comp->compId = FGuid();

		findC = components.find(comp->ComponentId());
	}

	components[comp->ComponentId()] = comp;
	if (world->IsActive())
		comp->OnStart();

	return comp;
}

CEntityComponent* CEntity::AddComponent(FClass* type, SizeType id)
{
	CEntityComponent* comp = (CEntityComponent*)CreateObject(type, name);
	if (!comp)
		return nullptr;

	comp->ent = this;
	comp->SetOwner(this);
	comp->Init();

	comp->compId = id;

	auto findC = components.find(comp->ComponentId());
	while (findC != components.end())
	{
		// re-generate the id
		comp->compId = FGuid();

		findC = components.find(comp->ComponentId());
	}

	components[comp->ComponentId()] = comp;
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
			if (comp.second->GetClass() == type && comp.second->Name() == name)
				return comp.second;
		}
	}
	else
	{
		for (auto& comp : components)
		{
			if (comp.second->GetClass() == type)
				return comp.second;
		}
	}

	return nullptr;
}

CEntityComponent* CEntity::GetComponent(SizeType id)
{
	for (auto& comp : components)
	{
		if (comp.first == id)
			return comp.second;
	}
	return nullptr;
}

void CEntity::RemoveComponent(CEntityComponent* comp)
{
	for (auto it = components.begin(); it != components.end(); it++)
	{
		if (it->second == comp)
		{
			components.erase(it);
			comp->Delete();
			break;
		}
	}
}

FOutputBinding& CEntity::AddOutput()
{
	boundOutputs.Add();
	return *boundOutputs.last();
}

FBounds CEntity::GetBounds()
{
	FBounds r;
	for (auto comp : components)
	{
		if (auto scene = Cast<CSceneComponent>(comp.second); scene)
			r = r.Combine(scene->Bounds());
	}
	return r;
}

void CEntity::Init()
{
	rootComponent = AddComponent<CSceneComponent>("root");
}

void CEntity::OnStart()
{
	for (auto& comp : components)
		comp.second->OnStart();

	outputOnStart();
}

void CEntity::OnStop()
{
}

void CEntity::Update(double dt)
{
	for (auto& comp : components)
		comp.second->Update(dt);
}

void CEntity::Serialize(FMemStream& out)
{
	BaseClass::Serialize(out);

	SizeType numComponents = components.size();
	out << &numComponents;

	for (auto& comp : components)
	{
		// Write component class typename
		out << comp.second->GetClass()->GetInternalName();

		// Write the components name. this already gets written by the default serializer
		// but we need it earlier in order to write to the correct component when loading.
		out << comp.second->Name();

		FMemStream compOut;
		comp.second->Serialize(compOut);

		SizeType id = comp.first;
		out << &id;

		bool bUserCreated = comp.second->IsUserCreated();
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
			CONSOLE_LogWarning("CEntity", "Serialized component has unkown class '" + classTypename + "', loading for entity " + Name() + ".");
			in.Seek(compDataSize, SEEK_CUR);
			continue;
		}

		CEntityComponent* comp = nullptr;
		if (!bUserCreated)
		{
			comp = GetComponent(compClass, compName);

			if (comp)
			{
				components.erase(comp->compId);
				components[compId] = comp;
			}
		}
		if (!comp)
			comp = AddComponent(compClass, compId);

		comp->compId = compId;
		comps[i].Key = comp;
		comps[i].Value.Resize(compDataSize);
		in.Read(comps[i].Value.Data(), compDataSize);
	}

	for (SizeType i = 0; i < numComponents; i++)
	{
		comps[i].Key->Load(comps[i].Value);
		comps[i].Key->SetOwner(this);
		if (comps[i].Key->bEditorOnly && !gIsEditor)
			RemoveComponent(comps[i].Key);
	}
}

void CEntity::OnDelete()
{
	auto comps = components;
	for (auto it = comps.rbegin(); it != comps.rend(); it++)
		it->second->Delete();
	components.clear();

	FWorldRegisterer::UnregisterEntity(world, this);
}

void CEntity::FireOutput(const FString& output)
{
	for (int i = 0; i < boundOutputs.Size(); i++)
	{
		if (boundOutputs[i].outputName == output)
		{
			if (boundOutputs[i].bOnlyOnce && boundOutputs[i].fireCount > 0)
				continue;

			GetWorld()->GetEntityIOManager()->FireEvent(this, i);
		}
	}
}
