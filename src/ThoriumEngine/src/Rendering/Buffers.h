#pragma once

#include <Util/Core.h>
#include "Object/Object.h"
#include "Buffers.generated.h"

enum EBufferFlags : uint
{
	TH_BUFFER_FLAGS_NONE = 0,
	TH_BUFFER_FLAGS_CPU_READ = 1 << 0,
	TH_BUFFER_FLAGS_CPU_WRITE = 1 << 1,
};

enum EBufferType
{
	TH_BUFFER_TYPE_INVALID = 0,
	TH_BUFFER_TYPE_VERTEX_BUFFER,
	TH_BUFFER_TYPE_INDEX_BUFFER,
	TH_BUFFER_TYPE_SHADER_BUFFER
};

struct FBufferDescriptor
{
	uint32 type;
	uint32 bufferSize;
	void* data;
	uint32 dataStride;

	uint32 flags;
};

CLASS(Abstract)
class ENGINE_API IGBuffer : public CObject
{
	GENERATED_BODY()

public:
	IGBuffer() = default;
	virtual ~IGBuffer() = default;

	virtual void Update(SizeType amount, void* data, SizeType offset = 0) = 0;

	inline const FBufferDescriptor& Descriptor() const { return desc; }

protected:
	FBufferDescriptor desc;
};

//CLASS(Abstract)
//class ENGINE_API IVertexBuffer : public CObject
//{
//	GENERATED_BODY()
//public:
//	IVertexBuffer() = default;
//	virtual ~IVertexBuffer() = default;
//
//	virtual void Update(SizeType amount, void* data, SizeType offset = 0) = 0;
//
//protected:
//	FBufferDescriptor desc;
//};
//
//CLASS(Abstract)
//class ENGINE_API IIndexBuffer : public CObject
//{
//	GENERATED_BODY()
//public:
//	IIndexBuffer() = default;
//	virtual ~IIndexBuffer() = default;
//
//	virtual void Update(SizeType amount, void* data, SizeType offset = 0) = 0;
//};
//
//CLASS(Abstract)
//class ENGINE_API IShaderBuffer : public CObject
//{
//	GENERATED_BODY()
//public:
//	virtual ~IShaderBuffer() = default;
//
//	virtual void Update(SizeType size, void* data) = 0;
//	//virtual void Update(SizeType offset, SizeType size, void* data) = 0;
//	inline SizeType Size() const { return size; }
//
//protected:
//	SizeType size;
//};
