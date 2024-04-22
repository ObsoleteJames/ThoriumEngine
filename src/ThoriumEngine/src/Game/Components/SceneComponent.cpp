
#include "SceneComponent.h"

FVector CSceneComponent::GetWorldPosition() const
{
	//if (parent)
	//	return (parent->GetWorldRotation().Rotate(position) * parent->GetWorldScale()) + parent->GetWorldPosition();
	//return position;

	return worldTransform.position;
}

FVector CSceneComponent::GetWorldScale() const
{
	/*if (parent)
		return scale * parent->GetWorldScale();
	return scale;*/

	return worldTransform.scale;
}

FQuaternion CSceneComponent::GetWorldRotation() const
{
	/*if (parent)
		return parent->GetWorldRotation() * rotation;
	return rotation;*/

	return worldTransform.rotation;
}

void CSceneComponent::SetWorldPosition(const FVector& pos)
{
	if (parent)
		position = pos - parent->GetWorldRotation().Rotate(parent->GetWorldPosition()); // TODO: use scaled position
	else
		position = pos;

	UpdateWorldTransform();
}

void CSceneComponent::SetWorldScale(const FVector& s)
{
	if (parent)
		scale = s / parent->GetWorldScale();
	else
		scale = s;

	UpdateWorldTransform();
}

void CSceneComponent::SetWorldRotation(const FQuaternion& rot)
{
	if (parent)
		rotation = rot * parent->GetWorldRotation().Conjugate();
	else
		rotation = rot;

	UpdateWorldTransform();
}

void CSceneComponent::SetPosition(const FVector& pos)
{
	position = pos;
	UpdateWorldTransform();
}

void CSceneComponent::SetScale(const FVector& s)
{
	scale = s;
	UpdateWorldTransform();
}

void CSceneComponent::SetRotation(const FQuaternion& q)
{
	rotation = q;
	UpdateWorldTransform();
}

void CSceneComponent::AttachTo(CSceneComponent* newParent, const FTransformSpace& space /*= FTransformSpace::KEEP_WORLD_TRANSFORM*/)
{
	if (parent == newParent || newParent == this)
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

	auto it = parent->children.Find(this);
	if (it != parent->children.end())
		parent->children.Erase(it);
	parent = nullptr;
}

FBounds CSceneComponent::Bounds() const
{
	FBounds r = bounds;

	FQuaternion rot = GetWorldRotation();
	FVector pos = GetWorldPosition();
	FVector scale = GetWorldScale();

	r.extents *= scale;

	r = r.Rotate(rot);
	//r.extents = (rot.Rotate(r.extents) * scale);
	//r.position = (rot.Rotate(r.position) * scale) + pos;
	//r.extents = r.extents * scale;
	r.position = r.position * scale + pos;
	return r;
}

void CSceneComponent::UpdateWorldTransform(bool bUpdateChildren)
{
	if (parent)
	{
		worldTransform.position = (parent->GetWorldRotation().Rotate(position) * parent->GetWorldScale()) + parent->GetWorldPosition();
		worldTransform.rotation = parent->GetWorldRotation() * rotation;
		worldTransform.scale = parent->GetWorldScale() * scale;
	}
	else
		worldTransform = FTransform(position, rotation, scale);

	if (bUpdateChildren)
	{
		for (auto& ch : children)
			if (ch)
				ch->UpdateWorldTransform();
	}
}

void CSceneComponent::OnDelete()
{
	BaseClass::OnDelete();

	if (parent)
		Detach();
}
