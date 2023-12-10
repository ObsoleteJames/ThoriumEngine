#pragma once

#include "EngineCore.h"
#include <Util/Assert.h>

/**
 * A LIFO Stack
 */
struct FStack
{
public:
	FStack(uint32 _size) : size(_size), rsp(0)
	{
		stack = (uint8*)malloc(_size);
	}

	~FStack()
	{
		free(stack);
	}

	template<typename T>
	inline void Pop(T& out)
	{
		THORIUM_ASSERT(rsp >= sizeof(T), "stack out of data");
		rsp -= sizeof(T);
		memcpy(&out, stack + rsp, sizeof(T));
	}

	template<typename T>
	inline void Push(const T& in)
	{
		THORIUM_ASSERT(rsp + sizeof(T) <= size, "stack out of data");
		memcpy(stack + rsp, &in, sizeof(T));
		rsp += sizeof(T);
	}

	void Push(void* data, SizeType size)
	{
		THORIUM_ASSERT(rsp + size <= size, "stack out of data");
		memcpy(stack + rsp, data, size);
		rsp += (uint32)size;
	}

private:
	uint32 size;
	uint32 rsp; // pointer to the top of the stack
	uint8* stack = nullptr;
};

#define STACK_POP_VAR(type, var, stack) type var; \
	stack.Pop(var)
