#pragma once

#include "Game/EntityComponent.h"
#include "Math/Vectors.h"
#include "Math/Bounds.h"
#include "SceneComponent.generated.h"

enum class FTransformSpace
{
	KEEP_WORLD_POSITION = 1 << 0,
	KEEP_WORLD_ROTATION = 1 << 1,
	KEEP_WORLD_SCALE = 1 << 2,

	KEEP_LOCAL_POSITION = 0,
	KEEP_LOCAL_ROTATION = 0,
	KEEP_LOCAL_SCALE = 0,

	KEEP_WORLD_TRANSFORM = KEEP_WORLD_POSITION | KEEP_WORLD_ROTATION | KEEP_WORLD_SCALE,
	KEEP_LOCAL_TRANSFORM = 0
};

CLASS()
class ENGINE_API CSceneComponent : public CEntityComponent
{
	GENERATED_BODY()

public:
	CSceneComponent() = default;

	FVector GetWorldPosition() const;
	FVector GetWorldScale() const;
	FQuaternion GetWorldRotation() const;

	void SetWorldPosition(const FVector& pos);
	void SetWorldScale(const FVector& scale);
	void SetWorldRotation(const FQuaternion& rot);

	inline void SetPosition(const FVector& pos) { position = pos; }
	inline void SetScale(const FVector& s) { scale = s; }
	inline void SetRotation(const FQuaternion& q) { rotation = q; }

	inline const FVector& GetPosition() const { return position; }
	inline const FVector& GetScale() const { return scale; }
	inline const FQuaternion& GetRotation() const { return rotation; }

	inline FVector GetForwardVector() const { return GetWorldRotation().Rotate({ 0.f, 0.f, 1.f }); }
	inline FVector GetRightVector() const { return GetWorldRotation().Rotate({ 1.f, 0.f, 0.f }); }
	inline FVector GetUpVector() const { return GetWorldRotation().Rotate({ 0.f, 1.f, 0.f }); }

	void AttachTo(CSceneComponent* newParent, const FTransformSpace& space = FTransformSpace::KEEP_WORLD_TRANSFORM);
	void Detach(const FTransformSpace& space = FTransformSpace::KEEP_WORLD_TRANSFORM);

	inline CSceneComponent* GetParent() const { return parent; }
	inline const TArray<TObjectPtr<CSceneComponent>>& GetChildren() const { return children; }

	FBounds Bounds() const;

private:
	PROPERTY()
	TObjectPtr<CSceneComponent> parent;

	PROPERTY()
	TArray<TObjectPtr<CSceneComponent>> children;

	PROPERTY()
	FVector position;

	PROPERTY()
	FVector scale = FVector(1.f);

	PROPERTY()
	FQuaternion rotation;

protected:
	FBounds bounds;

};
