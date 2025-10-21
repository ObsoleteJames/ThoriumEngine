#pragma once

#include <Util/Core.h>
#include "Object/ObjectMacros.h"
#include "RenderLayer.generated.h"

ENUM(Flags)
enum ERenderLayer : uint16
{
	R_LAYER_DEFAULT = 1,
	R_LAYER_CUBEMAP = 1 << 1, // render to reflection cubemaps
	R_LAYER_LIGHTMAP = 1 << 2, // contribute to GI lightmap
	R_LAYER_VIEWMODEL = 1 << 3, // used for rendering first person weapons and such.

	R_LAYER_EDITOR = 1 << 4, // editor only visuals.

	R_LAYER_DEBUG = 1 << 5, // used for debug rendering

	R_LAYER_CUSTOM0 = 1 << 6,
	R_LAYER_CUSTOM1 = 1 << 7,
	R_LAYER_CUSTOM2 = 1 << 8,
	R_LAYER_CUSTOM3 = 1 << 9,
	R_LAYER_CUSTOM4 = 1 << 10,
	R_LAYER_CUSTOM5 = 1 << 11,
	R_LAYER_CUSTOM6 = 1 << 12,
	R_LAYER_CUSTOM7 = 1 << 13,
	R_LAYER_CUSTOM8 = 1 << 14,
	R_LAYER_CUSTOM9 = 1 << 15,
};

#define R_LAYER_PRESET_DEFAULT R_LAYER_DEFAULT | R_LAYER_CUBEMAP | R_LAYER_LIGHTMAP
