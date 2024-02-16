#pragma once

#include <Util/Core.h>
#include "EngineCore.h"
#include "Object/Object.h"
#include "Game/World.h"
#include "Math/Vectors.h"
#include "Math/Bounds.h"
#include "Game/Components/SceneComponent.h"
#include "Object/ObjectHandle.h"
#include "Entity.generated.h"

class CEntityComponent;
class CSceneComponent;
class CEntity;

ENUM()
enum EEntityType
{
	ENTITY_STATIC = 1	META(Name = "Static"),
	ENTITY_DYNAMIC		META(Name = "Dynamic")
};

STRUCT()
struct FOutputBinding
{
	GENERATED_BODY()

	friend class CEntity;
	friend class CEntityIOManager;

public:
	PROPERTY()
	FString outputName;

	PROPERTY()
	TArray<uint8> arguments;

	PROPERTY()
	FObjectHandle targetObject;

	PROPERTY()
	FString targetInput;

	PROPERTY()
	float delay = 0.f;

	PROPERTY()
	bool bOnlyOnce = false;

protected:
	PROPERTY()
	int fireCount = 0;

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
			if (comp.second->GetClass() == T::StaticClass())
				arr.Add(comp);
	}

	inline const TMap<SizeType, TObjectPtr<CEntityComponent>>& GetAllComponents() const { return components; }

	CEntityComponent* AddComponent(FClass* type, const FString& name);
	CEntityComponent* GetComponent(FClass* type, const FString& name = "");

	CEntityComponent* AddComponent(FClass* type, SizeType id);
	CEntityComponent* GetComponent(SizeType id);

	void RemoveComponent(CEntityComponent* comp);

	inline CWorld* GetWorld() const { return world; }

	inline CSceneComponent* RootComponent() const { return rootComponent; }

	inline SizeType EntityId() const { return entityId; }

	FUNCTION()
	inline void SetWorldPosition(const FVector& p) { rootComponent->SetWorldPosition(p); }
	FUNCTION()
	inline void SetPosition(const FVector& p) { rootComponent->SetPosition(p); }

	FUNCTION()
	inline void SetWorldRotation(const FQuaternion& r) { rootComponent->SetWorldRotation(r); }
	FUNCTION()
	inline void SetRotation(const FQuaternion& r) { rootComponent->SetRotation(r); }

	FUNCTION()
	inline void SetWorldScale(const FVector& s) { rootComponent->SetWorldScale(s); }
	FUNCTION()
	inline void SetScale(const FVector& s) { rootComponent->SetScale(s); }

	inline FVector GetWorldPosition() const { return rootComponent->GetWorldPosition(); }
	inline FQuaternion GetWorldRotation() const { return rootComponent->GetWorldRotation(); }
	inline FVector GetWorldScale() const { return rootComponent->GetWorldScale(); }

	inline const TArray<FOutputBinding>& GetOutputs() const { return boundOutputs; }
	inline const FOutputBinding& GetOutput(SizeType index) const { return boundOutputs[index]; }
	FOutputBinding& AddOutput();

	FBounds GetBounds();

private:
	FUNCTION(Output, Name = "OnStart")
	void outputOnStart();

	FUNCTION()
	inline void Hide() { bIsVisible = false; }
	FUNCTION()
	inline void Show() { bIsVisible = true; }

	FUNCTION(Name = "Enable")
	inline void inputEnable() { bIsEnabled = true; }
	FUNCTION(Name = "Disable")
	inline void inputDisable() { bIsEnabled = false; }

	FUNCTION(Name = "SetHealth")
	inline void inputSetHealth(float health) { this->health = health; }

public:
	virtual void Serialize(FMemStream& out);
	virtual void Load(FMemStream& in);

protected:
	virtual void Init();
	virtual void OnStart();
	virtual void OnStop();
	virtual void Update(double dt);

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

	PROPERTY(Editable)
	EEntityType type = ENTITY_DYNAMIC;

	PROPERTY(Editable , Category = Health)
	bool bCanBeDamaged = false;

	PROPERTY(Editable, Category = Health)
	float health;

protected:
	CWorld* world;
	TObjectPtr<CSceneComponent> rootComponent;

	FGuid entityId;

private:
	PROPERTY()
	TArray<FOutputBinding> boundOutputs;

	TMap<SizeType, TObjectPtr<CEntityComponent>> components;

	bool bIsInitialized;

};
