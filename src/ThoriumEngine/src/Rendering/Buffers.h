#pragma once

#include <Util/Core.h>
#include "Object/Object.h"
#include "Buffers.generated.h"

CLASS(Abstract)
class ENGINE_API IVertexBuffer : public CObject
{
	GENERATED_BODY()
public:
	IVertexBuffer() = default;
	virtual ~IVertexBuffer() = default;

	virtual void Update(SizeType amount, void* data, SizeType offset = 0) = 0;
};

CLASS(Abstract)
class ENGINE_API IIndexBuffer : public CObject
{
	GENERATED_BODY()
public:
	IIndexBuffer() = default;
	virtual ~IIndexBuffer() = default;

	virtual void Update(SizeType amount, void* data, SizeType offset = 0) = 0;
};

CLASS(Abstract)
class ENGINE_API IShaderBuffer : public CObject
{
	GENERATED_BODY()
public:
	virtual ~IShaderBuffer() = default;

	virtual void Update(SizeType size, void* data) = 0;
	//virtual void Update(SizeType offset, SizeType size, void* data) = 0;
	inline SizeType Size() const { return size; }

protected:
	SizeType size;
};
