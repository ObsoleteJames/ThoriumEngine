#pragma once

#include "EngineCore.h"
#include "Class.h"
#include "PropertyHandler.h"
#include "Object.h"

class ENGINE_API FArrayType
{
public:
	void(*Add)(void* obj, void* data);
	void(*AddEmpty)(void* obj);
	void(*Erase)(void* obj, SizeType i);
	void(*Clear)(void* obj);
	void(*Resize)(void* obj, SizeType size);
	SizeType(*Size)(void* obj);
	SizeType(*Capacity)(void* obj);
	void*(*Data)(void* obj);
	void*(*At)(void* obj, SizeType i);
};

class FMapType
{
	// TODO: implement map type.
};

class ENGINE_API FDefaultPropertyHandler : public IPropertyHandler
{
public:
	FDefaultPropertyHandler(void* obj, const FProperty* property);
	FDefaultPropertyHandler(CObject* obj, const FProperty* property);
	
	void Serialize(FMemStream& out) override;
	void Load(FMemStream& in) override;

	void SetValue(void* value) override;
	void* GetValue() override;

	bool Equals(IPropertyHandler* other) override;
	bool Equals(void* valuePtr) override;
};

class ENGINE_API FObjectPtrPropertyHandler : public FDefaultPropertyHandler
{
public:
	FObjectPtrPropertyHandler(void* obj, const FProperty* property);
	FObjectPtrPropertyHandler(CObject* obj, const FProperty* property);

	void Serialize(FMemStream& out) final;
	void Load(FMemStream& in) final;
};

class ENGINE_API FClassPtrPropertyHandler : public FDefaultPropertyHandler
{
public:
	FClassPtrPropertyHandler(void* obj, const FProperty* property);

	void Serialize(FMemStream& out) final;
	void Load(FMemStream& in) final;
};

class ENGINE_API FStringPropertyHandler : public IPropertyHandler
{
public:
	FStringPropertyHandler(void* obj, const FProperty* property);

	void Serialize(FMemStream& out) final;
	void Load(FMemStream& in) final;

	void SetValue(void* value) final;
	void* GetValue() final;

	bool Equals(IPropertyHandler* other) final;
	bool Equals(void* valuePtr) final;
};

class ENGINE_API FArrayPropertyHandler : public IPropertyHandler
{
public:
	FArrayPropertyHandler(void* obj, const FProperty* property);
	virtual ~FArrayPropertyHandler();

	void Serialize(FMemStream& out) final;
	void Load(FMemStream& in) final;

	void SetValue(void* value) final;
	void* GetValue() final;

	bool Equals(IPropertyHandler* other) final;
	bool Equals(void* valuePtr) final;

	void Add(void* element);
	void Add();
	void Erase(SizeType i);
	void Clear();
	void Resize(SizeType size);
	SizeType Size();
	SizeType Capacity();
	void* Data();
	void* At(SizeType i);

	inline FProperty* GetTemplateProperty() const { return templateProperty; }

protected:
	FArrayType* arrayData;
	FProperty* templateProperty; // the property of the array element.
};

class ENGINE_API FStructPropertyHandler : public IPropertyHandler
{
public:
	FStructPropertyHandler(void* obj, const FProperty* property);
	FStructPropertyHandler(void* obj, FStruct* type);
	
	void Serialize(FMemStream& out) final;
	void Load(FMemStream& in) final;
	
	void SetValue(void* value) final;
	void* GetValue() final;

	bool Equals(IPropertyHandler* other) final;
	bool Equals(void* valuePtr) final;

	inline FStruct* GetStructType() const { return structType; }

	inline const FSerializeSettings& GetSerializeSettings() const { return serializeSettings; }
	inline void SetSerializeSettings(const FSerializeSettings& settings) { serializeSettings = settings; }

	inline void SetDefaultObject(void* obj) { cdo = obj; }

private:
	void* cdo = nullptr;
	FStruct* structType;
	FSerializeSettings serializeSettings;
};
