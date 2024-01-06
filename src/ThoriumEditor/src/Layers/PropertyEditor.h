#pragma once

#include "Layer.h"
#include "EngineCore.h"
#include "Math/Vectors.h"

class FStruct;
class FProperty;
class CEntity;
class CEntityComponent;

class CPropertyEditor : public CLayer
{
public:
	void OnUIRender() override;

	void RenderClassProperties(FStruct* type, SizeType offset);
	void RenderTransformEdit();

	void RenderVectorProperty(SizeType offset, bool bReadOnly);
	void RenderColorProperty(SizeType offset, bool bReadOnly);
	void RenderQuatProperty(SizeType offset, bool bReadOnly);

	void RenderProperty(uint type, const FProperty* prop, void** objects, int objCount, SizeType offset);

public:
	CEntityComponent* selectedComp = nullptr;
	CEntity* prevEnt = nullptr;

	FVector rotCache;

};
