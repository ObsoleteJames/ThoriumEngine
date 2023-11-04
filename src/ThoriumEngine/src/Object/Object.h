#pragma once

#include <Util/MemStream.h>
#include <Util/Event.h>

#include "Util/Guid.h"
#include "EngineCore.h"
#include "ObjectManager.h"
#include "Misc/Script.h"
#include "ObjectMacros.h"

#include "Object.generated.h"

#if IS_DEV
#define OBJECT_KEEP_REFERENCES 0
#else
#define OBJECT_KEEP_REFERENCES 0
#endif

class CObject;

ENGINE_API CObject* CreateObject(FClass* type, const FString& name = "");
ENGINE_API FString GetUniqueObjectName(FClass* type);

template<typename T>
inline T* CreateObject(const FString& name = "")
{
	static_assert(std::is_base_of<CObject, T>::value);
	return (T*)CreateObject(T::StaticClass(), name);
}

template<typename TTo, typename TFrom>
inline TTo* Cast(TFrom* obj)
{
	if (!obj)
		return nullptr;

	if (obj->GetClass()->CanCast(TTo::StaticClass()))
		return (TTo*)obj;

	return nullptr;
}

class ENGINE_API CObjectBase
{
	friend class CObjectManager;

public:
	virtual FClass* GetClass() const = 0;
	virtual FString GetModule() const = 0;
};

template<typename T>
class TObjectPtr
{
public:
	TObjectPtr() = default;
	TObjectPtr(T*);
	TObjectPtr(const TObjectPtr<T>&);
	~TObjectPtr();

	bool IsValid();
	bool IsValid() const;

	inline operator T*() const { return (T*)ptr; }
	inline TObjectPtr<T>& operator=(T*);
	inline TObjectPtr<T>& operator=(const TObjectPtr<T>&);

	template<typename T2>
	operator TObjectPtr<T2>() const { return TObjectPtr<T2>(Cast<T2>(ptr)); }

	bool operator==(const TObjectPtr<T>& other) { return ptr == other.ptr; }
	bool operator==(T* other) { return ptr == (CObject*)other; }

	T& operator*() { return *(T*)ptr; }
	T* operator->() { return (T*)ptr; }

	const T& operator*() const { return *(T*)ptr; }
	const T* operator->() const { return (T*)ptr; }

private:
	CObject* ptr = nullptr;
};

template<typename TTo, typename TFrom>
inline TObjectPtr<TTo> Cast(const TObjectPtr<TFrom>& obj)
{
	if (obj->GetClass()->CanCast(TTo::StaticClass()))
		return (TObjectPtr<TTo>)obj;

	return nullptr;
}

template<typename TTo, typename TFrom>
inline TObjectPtr<TTo> CastChecked(const TObjectPtr<TFrom>& obj)
{
	if (!obj)
		return nullptr;

	if (obj->GetClass()->CanCast(TTo::StaticClass()))
		return (TObjectPtr<TTo>)obj;

	return nullptr;
}

/**
 * Base class for all Objects.
 */
CLASS(Abstract)
class ENGINE_API CObject : public CObjectBase
{
	friend class CObjectManager;

	GENERATED_BODY()

public:
	CObject();
	virtual ~CObject();

	inline bool Delete() { return CObjectManager::DeleteObject(this); }

	inline bool IsMarkedForDeletion() { return bMarkedForDeletion; }

	// Pointer Functions
#if OBJECT_KEEP_REFERENCES
	void Connect(void* object = nullptr);
	void Disconnect(void* object = nullptr);
#else
	void Connect();
	void Disconnect();
#endif
	inline SizeType GetUserCount() const { return users; }

	inline void MakeIndestructible(bool b = true) { bIndestructible = b; }
	inline bool IsIndestructible() const { return bIndestructible; }

	inline void SetName(const FString& n) { name = n; }
	inline const FString& Name() const { return name; }

	inline SizeType Id() const { return id; }
	void SetId(SizeType _id);

	virtual void Serialize(FMemStream& out);
	virtual void Load(FMemStream& in);

	virtual void PostLoad() {}

	// Should be a networked function
	void SetOwner(CObject* owner);

	inline const TArray<TObjectPtr<CObject>>& GetChildren() const { return Children; }

	template<class T = CObject>
	TObjectPtr<T> GetOwner() const { return Owner ? Cast<T>(Owner) : nullptr; }

private:
	void SerializeProperty(FMemStream& out, uint type, const FProperty* p, SizeType offset, void* object);
	bool LoadProperty(FMemStream& in, uint type, const FProperty* p, SizeType offset, void* object);

protected:
	void SerializeProperties(FMemStream& out, FStruct* structType, void* object);
	void LoadProperties(FMemStream& in, FStruct* structType, void* object);

protected:
	virtual void OnDelete() {}

	FUNCTION(MulticastRpc)
	void OnNetDelete();

	template<typename T>
	void BindEvent(TEvent<>& event, void(T::*func)()) { eventBindings.Add({ event.Bind(this, func), event }); }

public:
	/**
	 * Should this object be recplicated to other clients
	 */
	PROPERTY(Editable, Category = "Networking")
	bool bReplicated = true;

	/**
	 * Determines whether this object has net priority over others.
	 */
	PROPERTY(Editable, Category = "Networking")
	uint8 NetPriority = 0;

protected:
	PROPERTY()
	FString name;

	/**
	 * Prevents this object from being garbage collected. can still be deleted with Delete().
	 */
	bool bIndestructible = false;

	PROPERTY()
	TObjectPtr<CObject> Owner;

	PROPERTY()
	TArray<TObjectPtr<CObject>> Children;

private:
	TArray<TPair<size_t, TEvent<>&>> eventBindings;

#if OBJECT_KEEP_REFERENCES
	TArray<void*> references; // A list of all 'TObjectPtr's that reference this object.
#endif

	FGuid id;

	TAtomic<uint64> users = 0;
	bool bMarkedForDeletion = false;
};

template<typename T>
bool TObjectPtr<T>::IsValid()
{
	if (!ptr)
		return false;

	if ((ptr)->IsMarkedForDeletion())
	{
#if OBJECT_KEEP_REFERENCES
		(ptr)->Disconnect(this);
#else
		(ptr)->Disconnect();
#endif
		ptr = nullptr;
		return false;
	}

	return true;
}

template<typename T>
bool TObjectPtr<T>::IsValid() const
{
	if (!ptr)
		return false;

	if ((ptr)->IsMarkedForDeletion())
		return false;

	return true;
}

template<typename T>
TObjectPtr<T>& TObjectPtr<T>::operator=(T* o)
{
	if (ptr)
#if OBJECT_KEEP_REFERENCES
		(ptr)->Disconnect(this);
#else
		(ptr)->Disconnect();
#endif

	ptr = (CObject*)o;
	if (ptr)
#if OBJECT_KEEP_REFERENCES
		(ptr)->Connect(this);
#else
		(ptr)->Connect();
#endif
	return *this;
}

template<typename T>
TObjectPtr<T>& TObjectPtr<T>::operator=(const TObjectPtr<T>& o)
{
	if (ptr)
#if OBJECT_KEEP_REFERENCES
		(ptr)->Disconnect(this);
#else
		(ptr)->Disconnect();
#endif

	if (!o.IsValid())
	{
		ptr = nullptr;
		return *this;
	}

	ptr = o.ptr;
	if (ptr)
#if OBJECT_KEEP_REFERENCES
		(ptr)->Connect(this);
#else
		(ptr)->Connect();
#endif
	return *this;
}

template<typename T>
TObjectPtr<T>::TObjectPtr(T* o) : ptr((CObject*)o)
{
	if (ptr)
#if OBJECT_KEEP_REFERENCES
	(ptr)->Connect(this);
#else
	(ptr)->Connect();
#endif
}

template<typename T>
TObjectPtr<T>::TObjectPtr(const TObjectPtr<T>& other) : ptr(other.ptr)
{
	if (ptr)
#if OBJECT_KEEP_REFERENCES
	(ptr)->Connect(this);
#else
	(ptr)->Connect();
#endif
}

template<typename T>
TObjectPtr<T>::~TObjectPtr()
{
	if (ptr)
#if OBJECT_KEEP_REFERENCES
	(ptr)->Disconnect(this);
#else
	(ptr)->Disconnect();
#endif
}
