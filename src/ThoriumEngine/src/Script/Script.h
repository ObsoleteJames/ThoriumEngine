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
	INT = 0xFF, // Interrupt, stops code execution
	NOP = 0x0F,
	RET,		// return from function.

	JMP, // jump
	JNP, // jump if

	ADD,
	SUB,
	MUL,
	DIV,

	SHIFT_LEFT,
	SHIFT_RIGHT,

	AND,
	OR,
	NOT,
	XOR,

	CALL, // call a function

	CAST, // object casting

	PUSH_VAR_A,	// immediate
	PUSH_VAR_B,	// from local var
	PUSH_VAR_C,	// from object variable
	PUSH_VAR_D,	// from function argument
	PUSH_VAR_E,	// from stack
	PUSH_VAR_F,	// from register
	PUSH_VAR_G, // push return value onto stack
	PUSH_VAR_H, // from object variable (for structs)

	POP_VAR_A, // pop into register
	POP_VAR_B, // pop into object variable
	POP_VAR_C, // pop into local var

	MK_VAR, // make a local variable
	RM_VAR, // remove local variable

	MOVE_A, // move immediate value into register
	MOVE_B, // move value from local variable into register  
	MOVE_C, // move value of variable from object into register
	MOVE_D, // move value in stack into register
	MOVE_E, // move value in register into register
	MOVE_F, // move value in register into object variable.
	MOVE_G, // move value in register into local variable.
	MOVE_H, // move immediate value into object variable.
	MOVE_I, // move immediate value into local variable.
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
