#pragma once

#include "Rendering/Buffers.h"
#include "Assets/ModelAsset.h"

#include <d3d11.h>

class DirectXBuffer : public IGBuffer
{
public:
	DirectXBuffer(const FBufferDescriptor& desc);
	virtual ~DirectXBuffer();

	void Update(SizeType ammount, void* data, SizeType offset);

public:
	ID3D11Buffer* buffer = nullptr;
};

//class DirectXVertexBuffer : public IVertexBuffer
//{
//public:
//	DirectXVertexBuffer(const TArray<FVertex>& vertices);
//	DirectXVertexBuffer(const TArray<FSkinnedVertex>& vertices);
//	DirectXVertexBuffer(SizeType bufferSize);
//	virtual ~DirectXVertexBuffer();
//
//	void Update(SizeType amount, void* data, SizeType offset /* = 0 */);
//
//public:
//	ID3D11Buffer* buffer;
//};
//
//class DirectXIndexBuffer : public IIndexBuffer
//{
//public:
//	DirectXIndexBuffer(const TArray<uint>& indices);
//	DirectXIndexBuffer(SizeType bufferSize);
//	virtual ~DirectXIndexBuffer();
//
//	void Update(SizeType amount, void* data, SizeType offset /* = 0 */);
//
//public:
//	ID3D11Buffer* buffer;
//};
//
//class DirectXShaderBuffer : public IShaderBuffer
//{
//public:
//	DirectXShaderBuffer(void* data, SizeType size);
//	virtual ~DirectXShaderBuffer();
//
//	virtual void Update(SizeType size, void* data);
//	//virtual void Update(SizeType offset, SizeType size, void* data);
//
//public:
//	ID3D11Buffer* buffer;
//
//};
