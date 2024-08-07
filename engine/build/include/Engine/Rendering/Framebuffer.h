#pragma once

#include "EngineCore.h"
#include "Texture.h"

class ENGINE_API IFrameBuffer : public IBaseTexture
{
public:
	virtual ~IFrameBuffer() = default;

	virtual void Resize(int width, int height) = 0;
	virtual void Clear(float r = 0.f, float g = 0.f, float b = 0.f, float a = 1.f) = 0;
};

enum EDepthBufferFormat
{
	TH_DBF_R16,
	TH_DBF_R32,
	TH_DBF_D24_S8
};

struct ENGINE_API FDepthBufferInfo
{
	int width;
	int height;
	EDepthBufferFormat format;
	int arraySize;
	bool bShaderResource;
};

class ENGINE_API IDepthBuffer
{
public:
	virtual ~IDepthBuffer() = default;

	virtual void Resize(int width, int height) = 0;
	virtual void Clear(float a = 1.f) = 0;

	inline void GetSize(int& w, int& h) const { w = info.width; h = info.height; }

protected:
	FDepthBufferInfo info;
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
