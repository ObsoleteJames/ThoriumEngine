#pragma once

#include "EngineCore.h"
#include "Object/Class.h"
#include <Util/Assert.h>

#define STACK_DEFAULT_SIZE 32 * 64

/**
 * A LIFO Stack
 */
struct FStack
{
public:
	FStack(uint32 _size = STACK_DEFAULT_SIZE);
	FStack(void* data, uint32 size);

	~FStack();

	template<typename T>
	inline void Pop(T* out)
	{
		THORIUM_ASSERT(rsp >= sizeof(T), "stack out of data");
		rsp -= sizeof(T);
		memcpy(out, stack + rsp, sizeof(T));
	}

	template<typename T>
	inline void Push(const T* in)
	{
		THORIUM_ASSERT(rsp + sizeof(T) <= size, "stack out of data");
		memcpy(stack + rsp, in, sizeof(T));
		rsp += sizeof(T);
	}

	inline bool HasRoom(SizeType amount) { return rsp + amount < size; }

	void Push(void* data, SizeType size);
	void Pop(void* outData, SizeType size);

	void PushString(const FString& str);
	FString PopString();

protected:
	uint32 size;
	uint32 rsp; // pointer to the top of the stack
	uint8* stack = nullptr;
	uint8 bOwnsStack = true;
};

#define STACK_POP_VAR(type, var, stack) type var; \
	stack.Pop(var)

struct FVariable
{
	FArgType type;
	size_t size;
	TArray<uint8_t> data;
};

struct FFrame : public FStack
{
public:
	FFrame(uint32 stackSize = STACK_DEFAULT_SIZE);
	FFrame(void* stackBuff, uint32 stackSize);

	void Clear();

	FVariable* AddVariable(EDataType type, SizeType typeId, SizeType size);
	FVariable* GetVariable(uint index);
	void RemoveVariables(uint amount = 1); // removes x amount of variables from the back of the array.

	void PushStackIndex();
	void PopStackIndex();

public:
	//uint64 registers[4];

	FVariable returnValue;
	uint64 lastOperationValue;

	// list of all the types on the stack, in order.
	TArray<FArgType> stackTypes;

private:
	TArray<FVariable> localVars;
	uint lvc = 0; // local vars count
	uint lvi = 0; // local vars index
};
