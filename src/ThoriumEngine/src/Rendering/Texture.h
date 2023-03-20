#pragma once

#include "Rendering/Renderer.h"

class ENGINE_API ITexture2D
{
public:
	virtual ~ITexture2D() = default;

	virtual void UpdateData(void* data, int mipmapLevel) = 0;

};

class ENGINE_API ITextureCube
{
public:
	virtual ~ITextureCube() = default;

};
