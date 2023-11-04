#pragma once

#include "Object/Object.h"

class CEntity;
class CModelComponent;

/**
 * Multiclass Object ptr
 */
template<class... T>
class TObjectSelector
{
public:
	TObjectSelector() = default;

	template<class T2>
	inline bool IsType() const { if (ptr.IsValid()) return ptr->GetClass()->CanCast(T2::StaticClass()); return false; }

	inline bool IsValid() const { return (ptr.IsValid()); }

	template<class T2>
	inline T2* Get() { if (IsValid()) return (T2)*ptr; return nullptr; }

	TObjectSelector<T...>& operator=(CObject* obj);
	
private:
	TObjectPtr<CObject> ptr;
};

template<class... T>
TObjectSelector<T...>& TObjectSelector<T...>::operator=(CObject* obj)
{
	constexpr size_t count = sizeof...(T);
	for (size_t i = 0; i < count; i++)
	{
		if (T[i]::StaticClass())
		{

		}
	}
	return *this;
}
