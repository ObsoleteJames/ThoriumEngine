#pragma once

#include "Rendering/Buffers.h"
#include "Assets/ModelAsset.h"

#include <d3d11.h>

class DirectXVertexBuffer : public IVertexBuffer
{
public:
	DirectXVertexBuffer(const TArray<FVertex>& vertices);
	virtual ~DirectXVertexBuffer();

public:
	ID3D11Buffer* buffer;
};

class DirectXIndexBuffer : public IIndexBuffer
{
public:
	DirectXIndexBuffer(const TArray<uint>& indices);
	virtual ~DirectXIndexBuffer();

public:
	ID3D11Buffer* buffer;
};

class DirectXShaderBuffer : public IShaderBuffer
{
public:
	DirectXShaderBuffer(void* data, SizeType size);
	virtual ~DirectXShaderBuffer();

	virtual void Update(SizeType size, void* data);
	//virtual void Update(SizeType offset, SizeType size, void* data);

public:
	ID3D11Buffer* buffer;

};
