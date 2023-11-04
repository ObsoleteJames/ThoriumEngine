#pragma once

#include "EngineCore.h"

class ENGINE_API IFrameBuffer
{
public:
	virtual ~IFrameBuffer() = default;

	virtual void Resize(int width, int height) = 0;
	virtual void Clear(float r = 0.f, float g = 0.f, float b = 0.f, float a = 1.f) = 0;
	
	inline void GetSize(int& w, int& h) const { w = width; h = height; }

protected:
	int width, height;
};

class ENGINE_API IDepthBuffer
{
public:
	virtual ~IDepthBuffer() = default;

	virtual void Clear(float a = 1.f) = 0;

	inline void GetSize(int& w, int& h) const { w = width; h = height; }

protected:
	int width, height;
};

class ENGINE_API ISwapChain
{
public:
	virtual ~ISwapChain() = default;

	virtual void Present(int bVSync, int flags) = 0;
	virtual IFrameBuffer* GetFrameBuffer() = 0;
	virtual IDepthBuffer* GetDepthBuffer() = 0;
	virtual void Resize(int width, int height) = 0;

};
