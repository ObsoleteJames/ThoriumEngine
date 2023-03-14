
#include "Object.h"
#include "Module.h"
#include "Resources/Asset.h"

const SizeType zero = 0;

CObject::CObject()
{
	CObjectManager::RegisterObject(this);
}

CObject::~CObject()
{
	for (auto eb : eventBindings)
		eb.Value.Remove(eb.Key);
}

#if OBJECT_KEEP_REFERENCES
void CObject::Connect(void* obj)
#else
void CObject::Connect()
#endif
{
	users++;
#if OBJECT_KEEP_REFERENCES
	references.Add(obj);
#endif
}

#if OBJECT_KEEP_REFERENCES
void CObject::Disconnect(void* obj)
#else
void CObject::Disconnect()
#endif
{
#if OBJECT_KEEP_REFERENCES
	auto it = references.Find(obj);
	if (it != references.end())
		references.Erase(it);
#endif
	users--;
}

void CObject::SetId(SizeType _id)
{
	SizeType old = id; 
	id = _id; 
	CObjectManager::IdChanged(this, old);
}

void CObject::Serialize(FMemStream& out)
{
	FClass* classObj = GetClass();

	//out << classObj->GetInternalName();
	out << &id;

	SerializeProperties(out, classObj, this);
}

void CObject::Load(FMemStream& in)
{
	in >> &id;

	LoadProperties(in, GetClass(), this);
}

void CObject::SerializeProperties(FMemStream& out, FStruct* structType, void* object)
{
	uint8 type = (structType->IsClass() ? ClassType_Class : ClassType_Struct);
	out << &type;
	out << structType->GetInternalName();

	SizeType numProp = structType->NumProperties();
	out << &numProp;

	const FProperty* p = structType->GetPropertyList();
	while (p)
	{
		out << &p->type;
		out << p->cppName;
		
		FMemStream data;

		SerializeProperty(data, p->type, p, p->offset, object);

		SizeType nextOffset = out.Tell() + data.Size();
		out << &nextOffset;
		out << data;

		p = p->next;
	}

	if (structType->IsClass() && ((FClass*)structType)->GetBaseClass())
		SerializeProperties(out, ((FClass*)structType)->GetBaseClass(), object);
}

void CObject::LoadProperties(FMemStream& in, FStruct* structType, void* object)
{
	uint8 type;
	FString structName;
	in >> &type;
	in >> structName;

	SizeType numProp;
	in >> &numProp;

	for (SizeType i = 0; i < numProp; i++)
	{
		uint type;
		FString name;
		SizeType nextOffset;

		in >> &type;
		in >> name;
		in >> &nextOffset;

		const FProperty* property = nullptr;
		for (const FProperty* p = structType->GetPropertyList(); p != nullptr; p = p->next)
		{
			if (p->type == type && p->cppName == name)
			{
				property = p;
				break;
			}
		}

		if (!property)
		{
			if (nextOffset > in.Size())
				break;

			in.Seek(nextOffset, SEEK_SET);
			continue;
		}

		if (!LoadProperty(in, type, property, property->offset, object))
			in.Seek(nextOffset, SEEK_SET);
	}

	if (structType->IsClass() && ((FClass*)structType)->GetBaseClass())
		LoadProperties(in, ((FClass*)structType)->GetBaseClass(), object);
}

void CObject::OnNetDelete_Implementation()
{

}

void CObject::SerializeProperty(FMemStream& data, uint type, const FProperty* p, SizeType offset, void* object)
{
	switch (type)
	{
	case EVT_STRUCT:
		if (FStruct* pType = CModuleManager::FindStruct(p->typeName); pType)
			SerializeProperties(data, pType, (void*)((SizeType)object + offset));
		else
			data << &zero;
		break;
	case EVT_CLASS:
	{
		TObjectPtr<CObject> ptr = *(CObject**)((SizeType)object + offset);
		if (ptr.IsValid())
			data << &ptr->id;
		else
			data << &zero;
	}
	break;
	case EVT_STRING:
	{
		if (p->typeName == "FString")
		{
			FString& str = *(FString*)((SizeType)object + offset);
			data << str;
		}
		else if (p->typeName == "WString")
		{
			WString& str = *(WString*)((SizeType)object + offset);
			data << str;
		}
	}
	break;
	case EVT_ENUM:
	{
		if (FEnum* pEnum = CModuleManager::FindEnum(p->typeName); pEnum)
			data.Write((void*)((SizeType)object + offset), pEnum->Size());
		else
			data.Write((void*)&zero, pEnum->Size());
	}
	break;
	case EVT_ARRAY:
	{
		FArrayHelper* helper = (FArrayHelper*)p->typeHelper;
		void* array = (void*)((SizeType)object + offset);

		SizeType arraySize = helper->Size(array);
		data << &arraySize;

		for (SizeType i = 0; i < arraySize; i++)
			SerializeProperty(data, helper->objType, p, 0, (void*)((SizeType)helper->Data(array) + (i * helper->objSize)));
	}
	break;
	case EVT_OBJECT_PTR:
	{
		TObjectPtr<CObject>& ptr = *(TObjectPtr<CObject>*)((SizeType)object + offset);
		bool bIsAsset = CModuleManager::FindClass(p->typeName)->CanCast(CAsset::StaticClass());
		//out << &bIsAsset;

		// This should be updated to use asset ids
		if (ptr.IsValid())
		{
			if (!bIsAsset)
				data << &ptr->id;
			else
				data << ((TObjectPtr<CAsset>)ptr)->GetPath();
		}
		else
		{
			if (bIsAsset)
			{
				wchar_t nul = L'\0';
				data << &nul;
			}
			else
				data << &zero;
		}
	}
	break;
	case EVT_CLASS_PTR:
	{
		FClass* ptr = *(FClass**)((SizeType)object + offset);
		if (ptr)
		{
			data << ptr->GetInternalName();
		}
		else
		{
			FString empty;
			data << empty;
		}
	}
		break;
	case EVT_FLOAT:
	{
		float& var = *(float*)((SizeType)object + offset);
		data << &var;
	}
	break;
	case EVT_DOUBLE:
	{
		double& var = *(double*)((SizeType)object + offset);
		data << &var;
	}
	break;
	case EVT_INT:
	{
		int64& var = *(int64*)((SizeType)object + offset);
		data.Write(&var, p->size);
	}
	break;
	case EVT_UINT:
	{
		uint64& var = *(uint64*)((SizeType)object + offset);
		data.Write(&var, p->size);
	}
	break;
	case EVT_BOOL:
	{
		bool& var = *(bool*)((SizeType)object + offset);
		data << &var;
	}
	break;
	}
}

bool CObject::LoadProperty(FMemStream& in, uint type, const FProperty* p, SizeType offset, void* object)
{
	switch (type)
	{
	case EVT_STRUCT:
	{
		if (FStruct* pType = CModuleManager::FindStruct(p->typeName); pType)
			LoadProperties(in, pType, (void*)((SizeType)object + offset));
		else
			return false;
	}
	break;
	case EVT_CLASS:
	{
		SizeType id;
		in >> &id;

		*(TObjectPtr<CObject>*)((SizeType)object + offset) = CObjectManager::FindObject(id);
	}
	break;
	case EVT_STRING:
	{
		if (p->typeName == "FString")
		{
			FString& str = *(FString*)((SizeType)object + offset);
			str.Clear();
			in >> str;
		}
		else if (p->typeName == "WString")
		{
			WString& str = *(WString*)((SizeType)object + offset);
			str.Clear();
			in >> str;
		}
		else
			return false;
	}
		break;
	case EVT_ENUM:
	{
		if (FEnum* pEnum = CModuleManager::FindEnum(p->typeName); pEnum)
			in.Read((void*)((SizeType)object + offset), pEnum->Size());
		else
			return false;
	}
		break;
	case EVT_ARRAY:
	{
		FArrayHelper* helper = (FArrayHelper*)p->typeHelper;
		void* array = (void*)((SizeType)object + offset);

		helper->Clear(array);

		SizeType arraySize;
		in >> &arraySize;

		for (SizeType i = 0; i < arraySize; i++)
		{
			helper->AddEmpty(array);
			LoadProperty(in, helper->objType, p, 0, (void*)((SizeType)helper->Data(array) + (i * helper->objSize)));
		}
	}
		break;
	case EVT_OBJECT_PTR:
	{
		TObjectPtr<CObject>& ptr = *(TObjectPtr<CObject>*)((SizeType)object + offset);
		FClass* cType = CModuleManager::FindClass(p->typeName);
		bool bIsAsset = cType->CanCast(CAsset::StaticClass());

		if (bIsAsset)
		{
			WString path;
			in >> path;

			if (!path.IsEmpty())
				ptr = (TObjectPtr<CObject>)CResourceManager::GetResource((FAssetClass*)cType, path);
		}
		else
		{
			SizeType id;
			in >> &id;

			ptr = CObjectManager::FindObject(id);
		}
	}
		break;
	case EVT_CLASS_PTR:
	{
		FClass*& ptr = *(FClass**)((SizeType)object + offset);

		FString className;
		in >> className;
		if (!className.IsEmpty())
			ptr = CModuleManager::FindClass(className);
	}
		break;
	case EVT_FLOAT:
	{
		float& var = *(float*)((SizeType)object + offset);
		in >> &var;
	}
	break;
	case EVT_DOUBLE:
	{
		double& var = *(double*)((SizeType)object + offset);
		in >> &var;
	}
	break;
	case EVT_INT:
	{
		int64& var = *(int64*)((SizeType)object + offset);
		in.Read(&var, p->size);
	}
	break;
	case EVT_UINT:
	{
		uint64& var = *(uint64*)((SizeType)object + offset);
		in.Read(&var, p->size);
	}
	break;
	case EVT_BOOL:
	{
		bool& var = *(bool*)((SizeType)object + offset);
		in >> &var;
	}
	break;
	}
	return true;
}

void CObject::SetOwner(CObject* obj)
{
	if (Owner == obj)
		return;

	if (Owner)
		Owner->Children.Erase(Owner->Children.Find(this));

	Owner = obj;
	if (Owner)
		Owner->Children.Add(this);
}

CObject* CreateObject(FClass* type, const FString& _name)
{
	if (type->Flags() & CTAG_ABSTRACT)
		return nullptr;

	CObject* obj = type->Instantiate();

	FString name = _name;
	if (name.IsEmpty())
		name = GetUniqueObjectName(type);

	obj->SetName(name);
	return obj;
}

FString GetUniqueObjectName(FClass* type)
{
	FString r = type->name;

	auto& objs = CObjectManager::GetAllObjects();
	int repeats = 0;
	for (auto it = objs.begin(); it != objs.end(); it++)
	{
		if (it->second->Name() == r)
		{
			if (repeats == 0)
				r += "_0";

			repeats++;
			r[r.Size() - 1] = '0' + repeats;

			it = objs.begin();
		}
	}

	return r;
}

