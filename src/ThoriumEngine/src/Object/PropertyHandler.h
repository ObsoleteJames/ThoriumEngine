#pragma once

#include <Util/MemStream.h>
#include "Class.h"

class ENGINE_API IPropertyHandler
{
public:
	IPropertyHandler(void* obj, const FProperty* p);
	IPropertyHandler(CObject* obj, const FProperty* p);
	virtual ~IPropertyHandler() = default;

	virtual void Serialize(FMemStream& out) = 0;
	virtual void Load(FMemStream& in) = 0;

	virtual void SetValue(void* value) = 0;
	virtual void* GetValue() = 0;

	virtual bool Equals(IPropertyHandler* other) = 0;
	virtual bool Equals(void* valuePtr) = 0;

	template<typename T>
	void SetValue(T& value)
	{
		SetValue(&value);
	}

	template<typename T>
	T GetValue()
	{
		T value = *(T*)GetValue();
		return value;
	}
	
	template<typename T>
	bool Equals(T value)
	{
		T otherValue = GetValue<T>();
		return Equals(&otherValue);
	}

	inline const FProperty* GetType() const { return type; }

protected:
	void* obj;
	void* valuePtr;
	const FProperty* type;
	bool bIsCObject = false; // whether or not obj is a CObject.
};
