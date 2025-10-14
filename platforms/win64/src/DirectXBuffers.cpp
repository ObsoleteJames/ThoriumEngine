
#include <Util/Assert.h>

#include "EngineCore.h"
#include "Platform/Windows/DirectX/DirectXBuffers.h"
#include "Platform/Windows/DirectX/DirectXInterface.h"
#include "Console.h"

DirectXVertexBuffer::DirectXVertexBuffer(const TArray<FVertex>& vertices)
{
	D3D11_BUFFER_DESC meshBufferInfo{};
	meshBufferInfo.Usage = D3D11_USAGE_DEFAULT;
	meshBufferInfo.ByteWidth = (uint)(vertices.Size() * sizeof(FVertex));
	meshBufferInfo.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	
	D3D11_SUBRESOURCE_DATA meshData{};
	meshData.pSysMem = vertices.Data();

	IRenderer::LockGPU();
	HRESULT hr = GetDirectXRenderer()->device->CreateBuffer(&meshBufferInfo, &meshData, &buffer);
	THORIUM_ASSERT(SUCCEEDED(hr), "Failed to create DirectX vertex buffer");
	IRenderer::UnlockGPU();
}

DirectXVertexBuffer::DirectXVertexBuffer(const TArray<FSkinnedVertex>& vertices)
{
	D3D11_BUFFER_DESC meshBufferInfo{};
	meshBufferInfo.Usage = D3D11_USAGE_DEFAULT;
	meshBufferInfo.ByteWidth = (uint)(vertices.Size() * sizeof(FSkinnedVertex));
	meshBufferInfo.StructureByteStride = sizeof(FSkinnedVertex);
	meshBufferInfo.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA meshData{};
	meshData.pSysMem = vertices.Data();

	IRenderer::LockGPU();
	HRESULT hr = GetDirectXRenderer()->device->CreateBuffer(&meshBufferInfo, &meshData, &buffer);
	THORIUM_ASSERT(SUCCEEDED(hr), "Failed to create DirectX vertex buffer");
	IRenderer::UnlockGPU();
}

DirectXVertexBuffer::DirectXVertexBuffer(SizeType bufferSize)
{
	D3D11_BUFFER_DESC meshBufferInfo{};
	meshBufferInfo.Usage = D3D11_USAGE_DYNAMIC;
	meshBufferInfo.ByteWidth = (uint)bufferSize;
	meshBufferInfo.StructureByteStride = sizeof(FVertex);
	meshBufferInfo.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	meshBufferInfo.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	void* data = malloc(bufferSize);
	memset(data, 0, bufferSize);

	D3D11_SUBRESOURCE_DATA meshData{};
	meshData.pSysMem = data;

	IRenderer::LockGPU();
	HRESULT hr = GetDirectXRenderer()->device->CreateBuffer(&meshBufferInfo, &meshData, &buffer);
	THORIUM_ASSERT(SUCCEEDED(hr), "Failed to create DirectX vertex buffer");
	IRenderer::UnlockGPU();

	free(data);
}

DirectXVertexBuffer::~DirectXVertexBuffer()
{
	if (buffer)
		buffer->Release();
}

void DirectXVertexBuffer::Update(SizeType amount, void* data, SizeType offset /* = 0 */)
{
	D3D11_MAPPED_SUBRESOURCE target;
	GetDirectXRenderer()->deviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &target);
	memcpy((void*)((SizeType)target.pData + offset), data, amount);
	GetDirectXRenderer()->deviceContext->Unmap(buffer, 0);
}

DirectXIndexBuffer::DirectXIndexBuffer(const TArray<uint>& indices)
{
	D3D11_BUFFER_DESC bufferInfo{};
	bufferInfo.Usage = D3D11_USAGE_DEFAULT;
	bufferInfo.ByteWidth = (uint)(indices.Size() * sizeof(uint));
	bufferInfo.BindFlags = D3D11_BIND_INDEX_BUFFER;
	
	D3D11_SUBRESOURCE_DATA data{};
	data.pSysMem = indices.Data();

	IRenderer::LockGPU();
	HRESULT hr = GetDirectXRenderer()->device->CreateBuffer(&bufferInfo, &data, &buffer);
	THORIUM_ASSERT(SUCCEEDED(hr), "Failed to create DirectX vertex buffer");
	IRenderer::UnlockGPU();
}

DirectXIndexBuffer::DirectXIndexBuffer(SizeType bufferSize)
{
	D3D11_BUFFER_DESC bufferInfo{};
	bufferInfo.Usage = D3D11_USAGE_DEFAULT;
	bufferInfo.ByteWidth = (uint)bufferSize;
	bufferInfo.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferInfo.BindFlags = D3D11_BIND_INDEX_BUFFER;

	void* data = malloc(bufferSize);
	memset(data, 0, bufferSize);

	D3D11_SUBRESOURCE_DATA meshData{};
	meshData.pSysMem = data;

	IRenderer::LockGPU();
	HRESULT hr = GetDirectXRenderer()->device->CreateBuffer(&bufferInfo, &meshData, &buffer);
	THORIUM_ASSERT(SUCCEEDED(hr), "Failed to create DirectX vertex buffer");
	IRenderer::UnlockGPU();

	free(data);
}

DirectXIndexBuffer::~DirectXIndexBuffer()
{
	if (buffer)
		buffer->Release();
}

void DirectXIndexBuffer::Update(SizeType amount, void* data, SizeType offset /* = 0 */)
{
	D3D11_MAPPED_SUBRESOURCE target;
	GetDirectXRenderer()->deviceContext->Map(buffer, 0, D3D11_MAP_WRITE, 0, &target);
	memcpy((void*)((SizeType)target.pData + offset), data, amount);
	GetDirectXRenderer()->deviceContext->Unmap(buffer, 0);
}

DirectXShaderBuffer::DirectXShaderBuffer(void* data, SizeType s)
{
	size = s;

	D3D11_BUFFER_DESC bufferInfo{};
	bufferInfo.Usage = D3D11_USAGE_DYNAMIC;
	bufferInfo.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferInfo.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferInfo.ByteWidth = (uint)(size % 16 == 0 ? size : size + (16 - (size % 16)));

	IRenderer::LockGPU();
	HRESULT hr = GetDirectXRenderer()->device->CreateBuffer(&bufferInfo, nullptr, &buffer);
	THORIUM_ASSERT(SUCCEEDED(hr), "Failed to create DirectX shader buffer");

	if (data)
	{
		D3D11_MAPPED_SUBRESOURCE target;
		GetDirectXRenderer()->deviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &target);
		memcpy(target.pData, data, size);
		GetDirectXRenderer()->deviceContext->Unmap(buffer, 0);
	}

	IRenderer::UnlockGPU();
}

DirectXShaderBuffer::~DirectXShaderBuffer()
{
	buffer->Release();
}

void DirectXShaderBuffer::Update(SizeType size, void* data)
{
	//IRenderer::LockGPU();
	D3D11_MAPPED_SUBRESOURCE target;
	GetDirectXRenderer()->deviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &target);
	memcpy(target.pData, data, size);
	GetDirectXRenderer()->deviceContext->Unmap(buffer, 0);
	//IRenderer::UnlockGPU();
}
