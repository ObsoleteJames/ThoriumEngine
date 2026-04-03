#pragma once

#include "Object/Class.h"
#include "Frame.h"

class CScriptModule;
class CThsClass;
struct FScriptFunction;

// 'this' identifier
#define THSCRIPT_THIS -2

enum class EThOpCode
{
	BRK = 0xFF, // Debug break
	NOP = 0x0F,
	RET,		// return from function.
	THROW,		// throw an exception.

	JMP, // jump
	JMP_EQ, // jump if non zero
	JMP_NQ, // jump if zero

	ADD,
	SUB,
	MUL,
	DIV,

	SHL, // shift left once
	SHR, // shift right once

	SHLI, // shift left by index
	SHRI, // shift right by index

	AND,
	OR,
	NOT,
	XOR,

	CMP_EQ, // compare equal
	CMP_NE, // compare not equal
	CMP_LT, // compare less than
	CMP_GT, // compare greater than
	CMP_LE, // compare less than or equal
	CMP_GE, // compare greater than or equal

	CONV_I1, // convert to int8
	CONV_I2, // convert to int16
	CONV_I4, // convert to int32
	CONV_I8, // convert to int64
	CONV_U1, // convert to uint8
	CONV_U2, // convert to uint16
	CONV_U4, // convert to uint32
	CONV_U8, // convert to uint64

	CONV_F4, // convert to float
	CONV_F8, // convert to double

	CALL, // call a function on object
	CALL_S, // call a static function
	CALL_I, // call function ptr

	CAST, // cast object to immediate type
	CAST_T, // cast object to type

	SIZEOF, // push the size of the type onto the stack
	TYPEOF, // push the type metadata ptr onto the stack

	LDARG, // load function argument[uint8 <i>] onto the stack
	LDARG_1, // load function argument 1 onto the stack
	LDARG_2, // load function argument 2 onto the stack
	LDARG_3, // load function argument 3 onto the stack
	LDARG_4, // load function argument 4 onto the stack
	LDARGA,  // load function argument adress onto the stack

	LDC_I4, // load immediate as int32 onto the stack
	LDC_I4_0, // load 0 as int32 onto the stack
	LDC_I4_1, // load 1 as int32 onto the stack
	LDC_I4_N1, // load -1 as int32 onto the stack
	
	LDC_I8, // load immediate as int64 onto the stack
	LDC_I8_0, // load 0 as int64 onto the stack
	LDC_I8_1, // load 1 as int64 onto the stack
	LDC_I8_N1, // load -1 as int64 onto the stack

	LDC_F4, // load immediate as float onto the stack
	LDC_F8, // load immediate as double onto the stack

	LDC_T, // load immediate as T onto the stack

	LDFLD, // load the field of an object onto the stack
	LDFLDA, // load the address of a field of an object onto the stack

	LDSFLD, // load static field onto the stack
	LDSFLDA, // load the address of a static field onto the stack

	LDLOC, // load local var[uint8 <i>] onto the stack
	LDLOC_1, // load local var 1 onto the stack
	LDLOC_2, // load local var 2 onto the stack
	LDLOC_3, // load local var 3 onto the stack
	LDLOC_4, // load local var 4 onto the stack

	LDSTR, // load immediate string onto the stack

	LDRV, // load the return value onto the stack
	LDOP, // load the value of the last operation onto the stack

	STARG, // pop value to the argument at index

	STFLD, // pop value to field of an object
	STSFLD, // pop value to static field

	STPTR, // pop value to memory at ptr
	STPTR_REF, // pop object ref into memory at ptr

	STLOC, // pop value into local variable at index
	STLOC_0, // pop value into local variable 1
	STLOC_1, // pop value into local variable 2
	STLOC_2, // pop value into local variable 3
	STLOC_3, // pop value into local variable 4
};

struct FThsProperty
{
	CThsClass* parent;

	FProperty* meta;
	FString defaultValue;
};

class CThsClass
{
	friend class CThScriptCompiler;

public:
	CThsClass() = default;

	inline FClass* GetType() const { return meta; }

private:
	void AddFunction(FScriptFunction* func);
	void AddProperty(const FThsProperty& p);

private:
	FClass* meta = nullptr;

	FScriptFunction* funcConstructor;
	FScriptFunction* funcDestructor;

	TArray<FThsProperty> properties;
	TArray<FScriptFunction*> functions;
};

class CThScript
{
	friend class CThScriptCompiler;
	friend class CScriptModule;

public:
	CThScript() = default;

	static int Exec(CObject* obj, const FFunction* func, FFrame& frame, const uint8_t* byteCode, size_t byteCodeSize);

	inline CScriptModule* GetModule() const { return module; }

protected:
	CScriptModule* module = nullptr;

	TArray<CThsClass*> classes;
	TArray<FStruct*> structs;
	TArray<FEnum*> enums;
};
