
#include "Frame.h"
#include "Math/Math.h"

FStack::FStack(uint32 _size) : size(_size), rsp(0)
{
	stack = (uint8*)malloc(_size);
}

FStack::~FStack()
{
	free(stack);
}

void FStack::Push(void* data, SizeType size)
{
	THORIUM_ASSERT(rsp + size <= size, "stack out of data");
	memcpy(stack + rsp, data, size);
	rsp += (uint32)size;
}

void FStack::Pop(void* outData, SizeType size)
{
	THORIUM_ASSERT(rsp >= size, "stack out of data");
	rsp -= size;
	memcpy(outData, stack + rsp, size);
}

void FStack::PushString(const FString& str)
{
	char ch = '\0';
	Push(&ch, 1);

	for (int i = str.Size(); i > 0; i--)
	{
		char ch = str[i - 1];
		Push(&ch, 1);
	}
}

FString FStack::PopString()
{
	FString r;
	char ch;
	do
	{
		Pop(&ch, 1);
		r += ch;
	} while (ch != '\0');

	return r;
}

FFrame::FFrame(uint32 stackSize) : FStack(stackSize)
{
}

void FFrame::Clear()
{
	localVars.Clear();
	rsp = 0;
}

FVariable* FFrame::AddVariable(EDataType type, SizeType typeId, SizeType size)
{
	localVars.Add();
	localVars.last()->data.Resize(size);
	lvc++;
	return localVars.last();
}

FVariable* FFrame::GetVariable(uint index)
{
	if (index + lvi < localVars.Size())
		return &localVars[index + lvi];
	return nullptr;
}

void FFrame::RemoveVariables(uint amount /*= 1*/)
{
	localVars.Erase(localVars.end() - FMath::Max(1u, amount), localVars.end());
	lvc -= amount;
}

void FFrame::PushStackIndex()
{
	Push(&lvi);
	lvi = lvc;
}

void FFrame::PopStackIndex()
{
	Pop(&lvi);
}
