
#include "SceneComponent.h"

FVector CSceneComponent::GetWorldPosition() const
{
	if (parent)
		return parent->GetRotation().Rotate(position) + parent->GetWorldPosition();
	return position;
}

FVector CSceneComponent::GetWorldScale() const
{
	if (parent)
		return scale * parent->GetWorldScale();
	return scale;
}

FQuaternion CSceneComponent::GetWorldRotation() const
{
	if (parent)
		return rotation * parent->GetWorldRotation();
	return rotation;
}

void CSceneComponent::SetWorldPosition(const FVector& pos)
{
	if (parent)
		position = pos - parent->GetRotation().Rotate(parent->GetWorldPosition()); // TODO: use rotational position
	else
		position = pos;
}

void CSceneComponent::SetWorldScale(const FVector& s)
{
	if (parent)
		scale = s * parent->GetWorldScale();
	else
		scale = s;
}

void CSceneComponent::SetWorldRotation(const FQuaternion& rot)
{
	rotation = rot;
}

void CSceneComponent::AttachTo(CSceneComponent* newParent, const FTransformSpace& space /*= FTransformSpace::KEEP_WORLD_TRANSFORM*/)
{
	if (parent == newParent)
		return;

	if (parent)
		Detach(space);

	parent = newParent;

	if ((int)space & (int)FTransformSpace::KEEP_WORLD_POSITION)
		position -= parent->GetRotation().Rotate(parent->GetWorldPosition());

	if ((int)space & (int)FTransformSpace::KEEP_WORLD_SCALE)
		scale /= parent->GetWorldScale();

	if ((int)space & (int)FTransformSpace::KEEP_WORLD_ROTATION)
		rotation *= parent->GetWorldRotation().Conjugate();

	parent->children.Add(this);
}

void CSceneComponent::Detach(const FTransformSpace& space /*= FTransformSpace::KEEP_WORLD_TRANSFORM*/)
{
	if (!parent)
		return;

	if ((int)space & (int)FTransformSpace::KEEP_WORLD_POSITION)
		position += parent->GetRotation().Rotate(parent->GetWorldPosition());

	if ((int)space & (int)FTransformSpace::KEEP_WORLD_SCALE)
		scale *= parent->GetWorldScale();

	if ((int)space & (int)FTransformSpace::KEEP_WORLD_ROTATION)
		rotation *= parent->GetWorldRotation();

	parent->children.Erase(parent->children.Find(this));
	parent = nullptr;
}
