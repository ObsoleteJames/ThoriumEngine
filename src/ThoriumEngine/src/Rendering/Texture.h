#pragma once

#include "Rendering/Renderer.h"

class ENGINE_API ITexture2D
{
public:
	virtual ~ITexture2D() = default;

};

class ENGINE_API ITextureCube
{
public:
	virtual ~ITextureCube() = default;

};
