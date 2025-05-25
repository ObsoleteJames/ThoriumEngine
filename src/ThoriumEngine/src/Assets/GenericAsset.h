#pragma once

#include "Asset.h"
#include "GenericAsset.generated.h"

/**
 * Base class type for generic assets.
 * Used to isolate generic assets from Thorium asset types.
 */
ASSET(Abstract)
class ENGINE_API CGenericAsset : public CAsset
{
	GENERATED_BODY()
};
