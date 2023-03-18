#pragma once

#include <Util/Core.h>
#include "EngineCore.h"
#include "Object/Object.h"
#include "Game/World.h"
#include "Math/Vectors.h"
#include "Game/Components/SceneComponent.h"
#include "Entity.generated.h"

class CEntityComponent;
class CSceneComponent;
class CEntity;

ENUM()
enum EEntityType
{
	ENTITY_STATIC = 1,
	ENTITY_DYNAMIC
};

STRUCT()
struct FOutputBinding
{
	GENERATED_BODY()

public:
	PROPERTY()
	FString outputName;

	PROPERTY()
	TArray<uint8> arguments;

	PROPERTY()
	TObjectPtr<CEntity> targetObject;

	PROPERTY()
	FString targetInput;

};

CLASS()
class ENGINE_API CEntity : public CObject
{
	GENERATED_BODY();

	friend class CScene;
	friend class CWorld;
	friend class CEntityComponent;
	
public:
	CEntity() = default;
	virtual ~CEntity() = default;

	template<typename T>
	inline T* AddComponent(const FString& name)
	{
		static_assert(std::is_base_of<CEntityComponent, T>::value);
		return (T*)AddComponent(T::StaticClass(), name);
	}

	template<typename T>
	inline T* GetComponent(const FString& name = "")
	{
		static_assert(std::is_base_of<CEntityComponent, T>::value);
		return (T*)GetComponent(T::StaticClass(), name);
	}

	template<typename T>
	void GetComponents(TArray<T*>& arr)
	{
		arr.Clear();
		for (auto& comp : components)
			if (comp->GetClass() == T::StaticClass())
				arr.Add(comp);
	}

	inline const TArray<TObjectPtr<CEntityComponent>>& GetAllComponents() const { return components; }

	CEntityComponent* AddComponent(FClass* type, const FString& name);
	CEntityComponent* GetComponent(FClass* type, const FString& name = "");

	void RemoveComponent(CEntityComponent* comp);

	inline CWorld* GetWorld() const { return world; }

	inline CSceneComponent* RootComponent() const { return rootComponent; }

	inline void SetWorldPosition(const FVector& p) { rootComponent->SetWorldPosition(p); }
	inline void SetPosition(const FVector& p) { rootComponent->SetPosition(p); }

	inline void SetWorldRotation(const FQuaternion& r) { rootComponent->SetWorldRotation(r); }
	inline void SetRotation(const FQuaternion& r) { rootComponent->SetRotation(r); }

	inline void SetWorldScale(const FVector& s) { rootComponent->SetWorldScale(s); }
	inline void SetScale(const FVector& s) { rootComponent->SetScale(s); }

	inline FVector GetWorldPosition() const { return rootComponent->GetWorldPosition(); }
	inline FQuaternion GetWorldRotation() const { return rootComponent->GetWorldRotation(); }
	inline FVector GetWorldScale() const { return rootComponent->GetWorldScale(); }

protected:
	virtual void Init();
	virtual void OnStart();
	virtual void OnStop();
	virtual void Update(double dt);

	virtual void Serialize(FMemStream& out);
	virtual void Load(FMemStream& in);

	virtual void OnDelete();

	void FireOutput(const FString& output);

public:
	PROPERTY(Editable, Category = Rendering)
	bool bIsVisible = true;

	PROPERTY()
	bool bIsEnabled = true;

	PROPERTY(Editable, Category = Rendering)
	bool bOwnerOnlySee;

	PROPERTY(Editable, Category = Rendering)
	bool bOwnerCantSee;

#if INCLUDE_EDITOR_DATA
	// Wether this entity should only be loaded in the editor.
	PROPERTY(Editable)
	bool bEditorOnly;

	// Wether this entity was created by the editor.
	// if true, this entity will not be serialized and will only be visible in the editor.
	bool bEditorEntity;
#endif

	PROPERTY()
	EEntityType type = ENTITY_DYNAMIC;

	PROPERTY(Editable , Category = Health)
	bool bCanBeDamaged = false;

	PROPERTY(Editable, Category = Health)
	float health;

protected:
	CWorld* world;
	TObjectPtr<CSceneComponent> rootComponent;

private:
	PROPERTY()
	TArray<FOutputBinding> boundOutputs;

	TArray<TObjectPtr<CEntityComponent>> components;

	bool bIsInitialized;

};
