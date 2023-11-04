#pragma once

#include "EngineCore.h"
#include "Object/Object.h"
#include "EntityComponent.generated.h"

class CEntity;
class CWorld;

CLASS(NotCreatable)
class ENGINE_API CEntityComponent : public CObject
{
	GENERATED_BODY()

	friend class CEntity;
	friend class CPropertiesWidget;

public:
	virtual void Init() {}

	virtual void OnStart() {}
	virtual void Update(double dt) {}
	
	inline TObjectPtr<CEntity> GetEntity() const { return ent; }
	CWorld* GetWorld() const;

	bool IsVisible() const;

	inline bool IsUserCreated() const { return bUserCreated; }

protected:
	virtual void OnDelete();
	
public:
	PROPERTY(Editable, Category = Rendering)
	bool bIsVisible = true;

	PROPERTY(Editable)
	bool bEditorOnly;

protected:
	TObjectPtr<CEntity> ent;

	PROPERTY()
	bool bUserCreated;

};
