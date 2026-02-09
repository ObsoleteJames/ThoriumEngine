
#include "ObjectManager.h"
#include "Object.h"
#include "Console.h"
#include "Module.h"

#include <shared_mutex>

TMap<FGuid, CObject*> CObjectManager::Objects;
TArray<CObject*> CObjectManager::ObjectsToDelete;

static std::shared_mutex objMutex;

CObject* CObjectManager::FindObject(const FString& name)
{
	std::shared_lock<std::shared_mutex> lock(objMutex);
	for (auto obj : Objects)
		if (obj.second->Name() == name)
			return obj.second;

	return nullptr;
}

CObject* CObjectManager::FindObject(SizeType id)
{
	std::shared_lock<std::shared_mutex> lock(objMutex);
	auto it = Objects.find(id);
	if (it == Objects.end())
		return nullptr;

	return it->second;
}

CObject* CObjectManager::FindObject(FClass* type)
{
	std::shared_lock<std::shared_mutex> lock(objMutex);
	for (auto obj : Objects)
		if (obj.second->GetClass() == type)
			return obj.second;

	return nullptr;
}

TArray<CObject*> CObjectManager::FindObjects(FClass* type)
{
	std::shared_lock<std::shared_mutex> lock(objMutex);
	TArray<CObject*> r;
	for (auto obj : Objects)
		if (obj.second->GetClass() == type)
			r.Add(obj.second);

	return r;
}

void CObjectManager::IdChanged(CObject* obj, SizeType oldId)
{
	std::unique_lock<std::shared_mutex> lock(objMutex);
	auto it = Objects.find(oldId);
	
	THORIUM_ASSERT(it != Objects.end(), "Failed to change object ID, original ID index couldn't be located!");

	Objects.erase(it);
	Objects[obj->Id()] = obj;
}

bool CObjectManager::DeleteObject(CObject* obj, bool bNoErase)
{
	if (obj->bMarkedForDeletion)
		return false;

	obj->bMarkedForDeletion = true;
	obj->OnDelete();

	std::unique_lock<std::shared_mutex> lock(objMutex);
	if (!bNoErase)
	{
		auto it = Objects.find(obj->id);
		if (it == Objects.end())
		{
			//CONSOLE_LogError("CObjectManager", "Attempted to delete object: " + obj->Name() + ", but object does not exist in the object database!");
			THORIUM_ASSERT(it != Objects.end(), "Attempted to delete object: " + obj->Name() + ", but object does not exist in the object database!");
			return false;
		}
		Objects.erase(it);
	}

	if (obj->users == 0)
	{
		delete obj;
		return true;
	}
	
	ObjectsToDelete.Add(obj);
	return true;
}

void CObjectManager::RegisterObject(CObject* obj)
{
	std::unique_lock<std::shared_mutex> lock(objMutex);
	for (auto it = Objects.find(obj->Id()); it != Objects.end(); it = Objects.find(obj->Id()))
		obj->id = FGuid();

	Objects[obj->id] = obj;
}

void CObjectManager::Update()
{
	std::unique_lock<std::shared_mutex> lock(objMutex);
	for (auto it = ObjectsToDelete.rbegin(); it != ObjectsToDelete.rend(); it++)
	{
		if (it->users == 0)
		{
			delete *it;
			ObjectsToDelete.Erase(it);
		}
	}

	//for (auto it = Objects.rbegin(); it != Objects.rend(); it++)
	//{
	//	if (it->second->users <= 0 && !it->second->bIndestructible)
	//	{
	//		DeleteObject(it->second, true);
	//		Objects.erase(it->first);
	//	}
	//}
	for (auto it = Objects.cbegin(); it != Objects.cend();)
	{
		if (it->second->users <= 0 && !it->second->bIndestructible)
		{
			lock.unlock();
			DeleteObject(it->second, true);
			lock.lock();
			Objects.erase(it++);
		}
		else
			++it;
	}
}

void CObjectManager::Shutdown()
{
	Update();

	for (auto it = ObjectsToDelete.rbegin(); it != ObjectsToDelete.rend(); it++)
		delete *it;

	for (auto it = Objects.rbegin(); it != Objects.rend(); it++)
	{
		// Clear all object pointers to prevent referencing deleted objects.
		CObject* obj = it->second;

		FClass* clas = obj->GetClass();
		while (clas)
		{
			const FProperty* prop = clas->GetPropertyList();
			while (prop)
			{
				if (prop->type == EVT_OBJECT_PTR)
				{
					*(SizeType*)((SizeType)obj + prop->offset) = 0;
				}

				prop = prop->next;
			}

			clas = clas->GetBaseClass();
		}

		delete it->second;
	}

	ObjectsToDelete.Clear();
	Objects.clear();
}

void CObjectManager::DeleteObjectsFromModule(CModule* module)
{
	auto objs = Objects;
	std::unique_lock<std::shared_mutex> lock(objMutex);
	for (auto obj : objs)
	{
		for (auto* c : module->Classes)
		{
			if (obj.second->GetClass() == c)
			{
				lock.unlock();
				delete obj.second;
				lock.lock();

				Objects.erase(obj.first);
				break;
			}
		}
	}

	for (auto it = ObjectsToDelete.rbegin(); it != ObjectsToDelete.rend(); it++)
	{
		for (auto* c : module->Classes)
		{
			if (it->GetClass() == c)
			{
				lock.unlock();
				delete *it;
				lock.lock();

				ObjectsToDelete.Erase(it);
				break;
			}
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
