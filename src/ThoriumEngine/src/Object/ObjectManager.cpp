
#include "ObjectManager.h"
#include "Object.h"

TMap<FGuid, CObject*> CObjectManager::Objects;
TArray<CObject*> CObjectManager::ObjectsToDelete;

CObject* CObjectManager::FindObject(const FString& name)
{
	for (auto obj : Objects)
		if (obj.second->Name() == name)
			return obj.second;

	return nullptr;
}

CObject* CObjectManager::FindObject(SizeType id)
{
	auto it = Objects.find(id);
	if (it == Objects.end())
		return nullptr;

	return it->second;
}

CObject* CObjectManager::FindObject(FClass* type)
{
	for (auto obj : Objects)
		if (obj.second->GetClass() == type)
			return obj.second;

	return nullptr;
}

TArray<CObject*> CObjectManager::FindObjects(FClass* type)
{
	TArray<CObject*> r;
	for (auto obj : Objects)
		if (obj.second->GetClass() == type)
			r.Add(obj.second);

	return r;
}

void CObjectManager::IdChanged(CObject* obj, SizeType oldId)
{
	Objects.erase(oldId);
	Objects[obj->Id()] = obj;
}

bool CObjectManager::DeleteObject(CObject* obj, bool bNoErase)
{
	if (obj->bMarkedForDeletion)
		return false;

	if (!bNoErase)
	{
		auto it = Objects.find(obj->id);
		if (it == Objects.end())
			return false;

		Objects.erase(it);
	}
	obj->OnDelete();

	if (obj->users == 0)
	{
		delete obj;
		return true;
	}
	
	ObjectsToDelete.Add(obj);

	obj->bMarkedForDeletion = true;
	return true;
}

void CObjectManager::RegisterObject(CObject* obj)
{
	Objects[obj->id] = obj;
}

void CObjectManager::Update()
{
	for (auto it = ObjectsToDelete.rbegin(); it != ObjectsToDelete.rend(); it++)
	{
		if (it->users == 0)
		{
			delete *it;
			ObjectsToDelete.Erase(it);
		}
	}

	for (auto it = Objects.rbegin(); it != Objects.rend(); it++)
	{
		if (it->second->users <= 0 && !it->second->bIndestructible)
		{
			DeleteObject(it->second, true);
			Objects.erase(std::next(it).base());
		}
	}
}

#if OBJECT_KEEP_REFERENCES
bool CObjectManager::ReplaceObject(CObject* target, CObject* replacement)
{
	auto it = Objects.find(target->id);
	if (it == Objects.end())
		return false;

	replacement->id = target->id;
	replacement->name = target->name;
	replacement->Owner = target->Owner;
	replacement->Children = target->Children;
	replacement->users = target->users.load();
	replacement->bIndestructible = target->bIndestructible;

	target->Owner = nullptr;
	
	for (auto it : target->references)
		memcpy(it, &replacement, sizeof(void*));

	delete target;
	return true;
}
#endif
