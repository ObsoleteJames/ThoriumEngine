
#include "PropertyTypes.h"
#include "Object.h"
#include "Assets/Asset.h"
#include "Game/Entity.h"
#include "Game/EntityComponent.h"
#include "Console.h"

IPropertyHandler::IPropertyHandler(void* obj, const FProperty* p) : obj(obj), type(p)
{
	if (p)
		valuePtr = (void*)((SizeType)obj + type->offset);
	else
		valuePtr = obj;
}

IPropertyHandler::IPropertyHandler(CObject* obj, const FProperty* p) : obj(obj), type(p), bIsCObject(true)
{
	if (p)
		valuePtr = (void*)((SizeType)obj + type->offset);
	else
		valuePtr = obj;
}

// --------- DEFAULT ---------
FDefaultPropertyHandler::FDefaultPropertyHandler(void* obj, const FProperty* property) : IPropertyHandler(obj, property)
{
}

FDefaultPropertyHandler::FDefaultPropertyHandler(CObject* obj, const FProperty* property) : IPropertyHandler(obj, property)
{
}

void FDefaultPropertyHandler::Serialize(FMemStream& out)
{
	out.Write(GetValue(), type->size);
}

void FDefaultPropertyHandler::Load(FMemStream& in)
{
	in.Read(GetValue(), type->size);
}

void FDefaultPropertyHandler::SetValue(void* value)
{
	memcpy(valuePtr, value, type->size);
}

void* FDefaultPropertyHandler::GetValue()
{
	return valuePtr;
}

bool FDefaultPropertyHandler::Equals(IPropertyHandler* other)
{
	THORIUM_ASSERT(other->GetType()->type == type->type, "FDefaultPropertyHandler::Equals() both properties must be the same type!");

	int i = memcmp(valuePtr, other->GetValue(), type->size);
	return i == 0;
}

bool FDefaultPropertyHandler::Equals(void* v)
{
	int i = memcmp(valuePtr, v, type->size);
	return i == 0;
}

// --------- TOBJECTPTR ---------
FObjectPtrPropertyHandler::FObjectPtrPropertyHandler(void* obj, const FProperty* property) : FDefaultPropertyHandler(obj, property)
{
}

FObjectPtrPropertyHandler::FObjectPtrPropertyHandler(CObject* obj, const FProperty* property) : FDefaultPropertyHandler(obj, property)
{
}

void FObjectPtrPropertyHandler::Serialize(FMemStream& out)
{
	const TObjectPtr<CObject>& ptr = *(TObjectPtr<CObject>*)valuePtr;

	EObjectPtrType type = OBJPTR_GENERIC;
	SizeType objId = 0;
	FClass* ptrClass = ptr.IsValid() ? ptr->GetClass() : CModuleManager::FindClass(this->type->templateType[0].typeName);
	if (ptrClass)
	{
		if (ptrClass->CanCast(CAsset::StaticClass()))
			type = OBJPTR_ASSET_REF;
		else if (ptrClass->CanCast(CEntity::StaticClass()))
		{
			type = OBJPTR_WORLD_ENTITY_REF;
			if (auto ent = CastChecked<CEntity>(ptr); ent)
				objId = ent->EntityId();
		}
		else if (ptrClass->CanCast(CEntityComponent::StaticClass()))
		{
			if (auto comp = CastChecked<CEntityComponent>(ptr); comp)
			{
				if (bIsCObject)
				{
					if (auto thisEnt = Cast<CEntity>((CObject*)obj); thisEnt && comp->GetEntity() == thisEnt)
						type = OBJPTR_ENTITY_COMP_REF;
					else if (auto thisComp = Cast<CEntityComponent>((CObject*)obj); thisComp && comp->GetEntity() == thisComp->GetEntity())
						type = OBJPTR_ENTITY_COMP_REF;
					else
						type = OBJPTR_WORLD_ENTITY_COMP_REF;
				}
				else
					type = OBJPTR_WORLD_ENTITY_COMP_REF;

				objId = comp->ComponentId();
			}
		}
	}

	out << &type;

	if (ptr.IsValid())
	{
		if (type == OBJPTR_GENERIC)
		{
			SizeType id = ptr->Id();
			out << &id;
		}
		else if (type == OBJPTR_ASSET_REF)
		{
			SizeType _id = ((TObjectPtr<CAsset>)ptr)->AssetId();
			out << &_id;
		}
		else if (type != OBJPTR_WORLD_ENTITY_COMP_REF)
			out << &objId;
		else
		{
			auto comp = Cast<CEntityComponent>(ptr);
			SizeType entId = comp->GetEntity()->EntityId();
			out << &entId;
			out << &objId;
		}
	}
	else
	{
		SizeType zero = 0;
		out << &zero;
	}
}

void FObjectPtrPropertyHandler::Load(FMemStream& in)
{
	TObjectPtr<CObject>& ptr = *(TObjectPtr<CObject>*)valuePtr;
	FClass* cType = CModuleManager::FindClass(type->templateType[0].typeName);

	EObjectPtrType type;
	in >> &type;

	if (type == OBJPTR_GENERIC)
	{
		SizeType id;
		in >> &id;

		ptr = CObjectManager::FindObject(id);
	}
	else if (type == OBJPTR_ASSET_REF)
	{
		SizeType id;
		in >> &id;

		ptr = (TObjectPtr<CObject>)CAssetManager::GetAsset((FAssetClass*)cType, id);
	}
	else if (type == OBJPTR_WORLD_ENTITY_REF)
	{
		CWorld* world = gWorld;
		if (bIsCObject)
			if (auto thisEnt = Cast<CEntity>((CObject*)obj); thisEnt)
				world = thisEnt->GetWorld();

		SizeType id;
		in >> &id;

		if (world)
			ptr = world->GetEntity(id);
	}
	else if (type == OBJPTR_WORLD_ENTITY_COMP_REF)
	{
		CWorld* world = gWorld;
		if (bIsCObject)
			if (auto thisEnt = Cast<CEntity>((CObject*)obj); thisEnt)
				world = thisEnt->GetWorld();

		SizeType entId;
		SizeType id;

		in >> &entId;
		in >> &id;

		if (world)
		{
			auto* ent = world->GetEntity(entId);
			if (ent)
				ptr = ent->GetComponent(id);
		}
	}
	else if (type == OBJPTR_ENTITY_COMP_REF)
	{
		SizeType id;
		in >> &id;

		if (bIsCObject)
		{
			if (auto thisEnt = Cast<CEntity>((CObject*)obj); thisEnt)
				ptr = thisEnt->GetComponent(id);
			else if (auto thisComp = Cast<CEntityComponent>((CObject*)obj); thisComp)
				ptr = thisComp->GetEntity()->GetComponent(id);
		}
	}
}

// --------- TCLASSPTR ---------
FClassPtrPropertyHandler::FClassPtrPropertyHandler(void* obj, const FProperty* property) : FDefaultPropertyHandler(obj, property)
{
}

void FClassPtrPropertyHandler::Serialize(FMemStream& out)
{
	FClass* ptr = *(FClass**)valuePtr;
	if (ptr)
	{
		out << ptr->GetInternalName();
	}
	else
	{
		FString empty;
		out << empty;
	}
}

void FClassPtrPropertyHandler::Load(FMemStream& in)
{
	FClass*& ptr = *(FClass**)valuePtr;

	FString className;
	in >> className;
	if (!className.IsEmpty())
		ptr = CModuleManager::FindClass(className);
}

// --------- FSTRING ---------
FStringPropertyHandler::FStringPropertyHandler(void* obj, const FProperty* property) : IPropertyHandler(obj, property)
{
}

void FStringPropertyHandler::Serialize(FMemStream& out)
{
	FString& str = *(FString*)valuePtr;
	out << str;
}

void FStringPropertyHandler::Load(FMemStream& in)
{
	FString& str = *(FString*)valuePtr;
	str.Clear();
	in >> str;
}

void FStringPropertyHandler::SetValue(void* value)
{
	FString* a = (FString*)valuePtr;
	FString* b = (FString*)value;
	*a = *b;
}

void* FStringPropertyHandler::GetValue()
{
	return valuePtr;
}

bool FStringPropertyHandler::Equals(IPropertyHandler* other)
{
	THORIUM_ASSERT(other->GetType()->type == type->type, "FDefaultPropertyHandler::Equals() both properties must be the same type!");

	FString* a = (FString*)valuePtr;
	FString* b = (FString*)other->GetValue();
	return *a == *b;
}

bool FStringPropertyHandler::Equals(void* v)
{
	FString* a = (FString*)valuePtr;
	FString* b = (FString*)v;
	return *a == *b;
}

// --------- TARRAY ---------
FArrayPropertyHandler::FArrayPropertyHandler(void* obj, const FProperty* property) : IPropertyHandler(obj, property)
{
	arrayData = (FArrayType*)type->typeHandler;

	FArgType& templateType = property->templateType[0];
	templateProperty = new FProperty();
	templateProperty->typeName = templateType.typeName;
	templateProperty->type = templateType.type;
	templateProperty->size = templateType.size;
	templateProperty->numTemplates = templateType.numTemplates;
	templateProperty->templateType = templateType.templateType;
	templateProperty->meta = nullptr;
	templateProperty->offset = 0;
	templateProperty->flags = 0;
	templateProperty->protectionLvl = 2;
}

FArrayPropertyHandler::~FArrayPropertyHandler()
{
	delete templateProperty;
}

void FArrayPropertyHandler::Serialize(FMemStream& out)
{
	SizeType size = Size();
	out << &size;

	for (SizeType i = 0; i < size; i++)
	{
		void* elementPtr = At(i);
		TUniquePtr<IPropertyHandler> handler = templateProperty->GetHandler(elementPtr);
		handler->Serialize(out);
	}
}

void FArrayPropertyHandler::Load(FMemStream& in)
{
	SizeType size;
	in >> &size;

	Resize(size);
	for (SizeType i = 0; i < size; i++)
	{
		void* elementPtr = At(i);
		TUniquePtr<IPropertyHandler> handler = templateProperty->GetHandler(elementPtr);
		handler->Load(in);
	}
}

void FArrayPropertyHandler::SetValue(void* value)
{
	CONSOLE_LogError("FArrayPropertyHandler", "Cannot set value of TArray property");
}

void* FArrayPropertyHandler::GetValue()
{
	return valuePtr;
}

bool FArrayPropertyHandler::Equals(IPropertyHandler* other)
{
	SizeType size = Size();

	if (size != ((FArrayPropertyHandler*)other)->Size())
		return false;

	for (SizeType i = 0; i < size; i++)
	{
		void* elementPtr = At(i);
		TUniquePtr<IPropertyHandler> handler = templateProperty->GetHandler(elementPtr);
		if (handler->Equals(((FArrayPropertyHandler*)other)->At(i)))
			continue;
		return false;
	}

	return true;
}

bool FArrayPropertyHandler::Equals(void* valuePtr)
{
	SizeType size = Size();
	FArrayPropertyHandler* other = GetType()->GetHandler<FArrayPropertyHandler>(valuePtr);
	return Equals(other);
}

void FArrayPropertyHandler::Add(void* element)
{
	arrayData->Add(valuePtr, element);
}

void FArrayPropertyHandler::Add()
{
	arrayData->AddEmpty(valuePtr);
}

void FArrayPropertyHandler::Erase(SizeType i)
{
	arrayData->Erase(valuePtr, i);
}

void FArrayPropertyHandler::Clear()
{
	arrayData->Clear(valuePtr);
}

void FArrayPropertyHandler::Resize(SizeType size)
{
	arrayData->Resize(valuePtr, size);
}

SizeType FArrayPropertyHandler::Size()
{
	return arrayData->Size(valuePtr);
}

SizeType FArrayPropertyHandler::Capacity()
{
	return arrayData->Capacity(valuePtr);
}

void* FArrayPropertyHandler::Data()
{
	return arrayData->Data(valuePtr);
}

void* FArrayPropertyHandler::At(SizeType i)
{
	return arrayData->At(valuePtr, i);
}

// --------- FSTRUCT ---------
FStructPropertyHandler::FStructPropertyHandler(void* obj, const FProperty* property) : IPropertyHandler(obj, property)
{
	structType = CModuleManager::FindStruct(property->typeName);
}

FStructPropertyHandler::FStructPropertyHandler(void* obj, FStruct* type) : IPropertyHandler(obj, nullptr), structType(type)
{
}

void FStructPropertyHandler::Serialize(FMemStream& out)
{
	uint8 type = (structType->IsClass() ? ClassType_Class : ClassType_Struct);
	out << &type;
	out << structType->GetInternalName();

	SizeType numProp = 0;
	const FProperty** propList = (const FProperty**)alloca(sizeof(FProperty*) * structType->NumProperties());

	void* defaultObj = cdo;
	if (!defaultObj)
	{
		if (structType->IsClass())
			defaultObj = ((FClass*)structType)->GetDefaultObject();
		else
			defaultObj = structType->GetDefaultObject();
	}

	const FProperty* prop = structType->GetPropertyList();
	while (prop)
	{
		bool bSerialize = prop->flags & VTAG_SERIALIZABLE;
		bool bStatic = prop->flags & VTAG_STATIC;
		if (!bSerialize || bStatic)
		{
			prop = prop->next;
			continue;
		}

		if (serializeSettings.bSaveGame && (prop->flags & VTAG_SAVEGAME) == 0)
		{
			prop = prop->next;
			continue;
		}

		if (!serializeSettings.bSerializeAllProperties && defaultObj && (prop->flags & VTAG_IGNORE_DEFAULT) == 0) // if we can't get the default object, we serialize it anyway.
		{
			TUniquePtr<IPropertyHandler> handler = prop->GetHandler(valuePtr);

			// if the property value is the same as the default value, we can skip serializing it.
			if (handler->Equals((void*)((SizeType)defaultObj + prop->offset)))
			{
				prop = prop->next;
				continue;
			}
		}

		propList[numProp] = prop;
		numProp++;
		prop = prop->next;
	}

	out << &numProp;

	for (SizeType i = 0; i < numProp; i++)
	{
		const FProperty* p = propList[i];

		// the old serialization format saved the type as a uint, so we keep doing that for backwards compatibility.	
		uint type = p->type;
		out << &type;
		out << p->cppName;

		FMemStream data;
		TUniquePtr<IPropertyHandler> handler = p->GetHandler(valuePtr);
		if (p->type == EVT_STRUCT)
		{
			if (p->flags & VTAG_IGNORE_DEFAULT)
			{
				FSerializeSettings s = serializeSettings;
				s.bSerializeAllProperties = true;
				((FStructPropertyHandler*)handler.Get())->SetSerializeSettings(s);
			}
			else
				((FStructPropertyHandler*)handler.Get())->SetSerializeSettings(serializeSettings);

			if (defaultObj)
			{
				void* defaultPtr = GetType() ? (void*)((SizeType)defaultObj + GetType()->offset) : (void*)defaultObj;
				((FStructPropertyHandler*)handler.Get())->SetDefaultObject(defaultPtr);
			}
		}

		handler->Serialize(data);

		SizeType dataSize = data.Size();
		out << &dataSize;
		out << data;
	}
}

void FStructPropertyHandler::Load(FMemStream& in)
{
	uint8 __type;
	in >> &__type;

	FString structName;
	in >> structName;

	SizeType numProp;
	in >> &numProp;

	for (SizeType i = 0; i < numProp; i++)
	{
		uint type;
		FString name;
		SizeType dataSize;

		in >> &type;
		in >> name;
		in >> &dataSize;

		const FProperty* property = structType->GetProperty(name);
		if (!property)
		{
			if (dataSize + in.Tell() > in.Size())
				break;

			in.Seek(dataSize, SEEK_CUR);
			continue;
		}

		SizeType nextOffset = dataSize + in.Tell();
		
		TUniquePtr<IPropertyHandler> handler = property->GetHandler(valuePtr);
		handler->Load(in);
	}
}

void FStructPropertyHandler::SetValue(void* value)
{
	CONSOLE_LogError("FStructPropertyHandler", "Cannot set value of FStruct property");
}

void* FStructPropertyHandler::GetValue()
{
	return valuePtr;
}

bool FStructPropertyHandler::Equals(IPropertyHandler* other)
{
	const FProperty* prop = structType->GetPropertyList();
	while (prop)
	{
		TUniquePtr<IPropertyHandler> handler = prop->GetHandler(valuePtr);
		TUniquePtr<IPropertyHandler> otherHandler = prop->GetHandler(other->GetValue());
		if (!handler->Equals(otherHandler.Get()))
			return false;
		prop = prop->next;
	}
	return true;
}

bool FStructPropertyHandler::Equals(void* v)
{
	const FProperty* prop = structType->GetPropertyList();
	while (prop)
	{
		TUniquePtr<IPropertyHandler> handler = prop->GetHandler(valuePtr);
 		TUniquePtr<IPropertyHandler> otherHandler = prop->GetHandler(v);
		if (!handler->Equals(otherHandler.Get()))
			return false;
		prop = prop->next;
	}
	return true;
}
