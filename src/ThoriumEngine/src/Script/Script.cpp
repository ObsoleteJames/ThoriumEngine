
#include "Script.h"
#include "Console.h"
#include "Object/Object.h"
#include "Object/ObjectManager.h"

#define SCRIPT_LogError(msg) CONSOLE_LogError("ElectronRuntime", func->cppName + " - " + msg)
#define SCRIPT_LogWarning(msg) CONSOLE_LogWarning("ElectronRuntime", func->cppName + " - " + msg)
#define SCRIPT_LogInfo(msg) CONSOLE_LogInfo("ElectronRuntime", func->cppName + " - " + msg)

//switch ()
//{
//case EVT_NULL:
//	break;
//case EVT_VOID:
//	break;
//case EVT_STRUCT:
//	break;
//case EVT_CLASS:
//	break;
//case EVT_STRING:
//	break;
//case EVT_ENUM:
//	break;
//case EVT_ARRAY:
//	break;
//case EVT_OBJECT_PTR:
//	break;
//case EVT_CLASS_PTR:
//	break;
//case EVT_STRUCT_PTR:
//	break;
//case EVT_ENUM_PTR:
//	break;
//case EVT_FLOAT:
//	break;
//case EVT_DOUBLE:
//	break;
//case EVT_INT:
//	break;
//case EVT_INT8:
//	break;
//case EVT_INT16:
//	break;
//case EVT_INT64:
//	break;
//case EVT_UINT:
//	break;
//case EVT_UINT8:
//	break;
//case EVT_UINT16:
//	break;
//case EVT_UINT64:
//	break;
//case EVT_BOOL:
//	break;
//}

bool PopIntoVariable(size_t& pc, FVariable& var, FFrame& stack, const uint8_t* byteCode)
{
	switch (var.type)
	{
	case EVT_OBJECT_PTR:
		var.data.Resize(sizeof(SizeType));
		var.size = 8;
		stack.Pop(var.data.Data(), sizeof(SizeType));
		break;
	case EVT_VOID:
		break;
	case EVT_STRUCT:
	{
		pc++;
		uint64 typeId = *(uint64*)&byteCode[pc];
		FStruct* type = CModuleManager::GetStruct(typeId);
		if (!type)
		{
			CONSOLE_LogError("ElectronRuntime", "Invalid struct id (" + FString::ToString(typeId) + ")!");
			return false;
		}
		var.typeId = typeId;
		var.size = type->Size();
		var.data.Resize(var.size);
		stack.Pop(var.data.Data(), var.size);
	}
	break;
	case EVT_CLASS:
		break;
	case EVT_ENUM:
		pc++;
		uint64 typeId = *(uint64*)&byteCode[pc];
		FEnum* type = CModuleManager::GetEnum(typeId);
		if (!type)
		{
			CONSOLE_LogError("ElectronRuntime", "Invalid enum id (" + FString::ToString(typeId) + ")!");
			return false;
		}
		var.typeId = typeId;
		var.size = type->Size();
		var.data.Resize(var.size);
		stack.Pop(var.data.Data(), var.size);
		break;
	case EVT_ARRAY:
		break;

	case EVT_STRING:
	{
		FString v = stack.PopString();
		var.data.Resize(v.Size());
		memcpy(var.data.Data(), v.Data(), v.Size());
	}
	break;

	case EVT_CLASS_PTR:
		var.size = 8;
		var.data.Resize(var.size);
		stack.Pop(var.data.Data(), 8);
		break;
	case EVT_STRUCT_PTR:
		var.size = 8;
		var.data.Resize(var.size);
		stack.Pop(var.data.Data(), 8);
		break;
	case EVT_ENUM_PTR:
		var.size = 8;
		var.data.Resize(var.size);
		stack.Pop(var.data.Data(), 8);
		break;

	case EVT_FLOAT:
		var.size = 4;
		var.data.Resize(var.size);
		stack.Pop(var.data.Data(), 4);
		break;

	case EVT_DOUBLE:
		var.size = 8;
		var.data.Resize(var.size);
		stack.Pop(var.data.Data(), 8);
		break;

	case EVT_INT:
		var.size = 4;
		var.data.Resize(var.size);
		stack.Pop(var.data.Data(), var.size);
		break;
	case EVT_INT8:
		var.size = 1;
		var.data.Resize(var.size);
		stack.Pop(var.data.Data(), var.size);
		break;
	case EVT_INT16:
		var.size = 2;
		var.data.Resize(var.size);
		stack.Pop(var.data.Data(), var.size);
		break;
	case EVT_INT64:
		var.size = 8;
		var.data.Resize(var.size);
		stack.Pop(var.data.Data(), var.size);
		break;

	case EVT_UINT:
		var.size = 4;
		var.data.Resize(var.size);
		stack.Pop(var.data.Data(), var.size);
		break;
	case EVT_UINT8:
		var.size = 1;
		var.data.Resize(var.size);
		stack.Pop(var.data.Data(), var.size);
		break;
	case EVT_UINT16:
		var.size = 2;
		var.data.Resize(var.size);
		stack.Pop(var.data.Data(), var.size);
		break;
	case EVT_UINT64:
		var.size = 8;
		var.data.Resize(var.size);
		stack.Pop(var.data.Data(), var.size);
		break;

	case EVT_BOOL:
		var.size = 1;
		var.data.Resize(var.size);
		stack.Pop(var.data.Data(), var.size);
		break;
	}

	return true;
}

int CThScript::Exec(CObject* obj, const FFunction* func, FFrame& stack, const uint8_t* byteCode, size_t byteCodeSize)
{
	size_t pc = 0; // program counter
	stack.PushStackIndex();

	while (pc < byteCodeSize)
	{
		EThOpCode opCode;
		opCode = (EThOpCode)byteCode[pc];

		if (opCode == EThOpCode::RET)
		{
			FVariable& rv = stack.returnValue;

			rv.type = func->returnType.type;

			if (!PopIntoVariable(pc, rv, stack, byteCode))
			{
				SCRIPT_LogError("OPERATION:RET unable to obtain return value!");
				return 1;
			}

			break;
		}

		// interrupt!
		if (opCode == EThOpCode::INT)
		{
			SCRIPT_LogInfo("Script Interrupt!");
			return 1;
		}

		if (opCode == EThOpCode::JMP)
		{
			pc++;
			uint64 jumpTarget = *(uint64*)&byteCode[pc];
			pc = jumpTarget;
			continue;
		}

		if (opCode == EThOpCode::JNP)
		{
			pc++;
			uint64 jumpTarget = *(uint64*)&byteCode[pc];

			if (stack.lastOperationValue)
				pc = jumpTarget;
			else
				pc++;
			continue;
		}

		if (opCode == EThOpCode::PUSH_VAR_A)
		{
			pc++;
			EDataType type;
			type = *(EDataType*)&byteCode[pc];
			pc++;

			switch (type)
			{
			case EVT_OBJECT_PTR:
			{
				uint64 ptr = *(uint64*)&byteCode[pc];
				pc += sizeof(uint64);
				stack.Push(&ptr);
			}
				break;
			case EVT_STRUCT:
				break;
			case EVT_ENUM:
			{
				uint64 typeId = *(uint64*)&byteCode[pc];
				pc += sizeof(uint64);
				FEnum* type = CModuleManager::GetEnum(typeId);
				if (!type)
				{
					SCRIPT_LogError("OPERATIONT:PUSH_VAR invalid enum type (" + FString::ToString(typeId) + ")!");
					return 1;
				}

				stack.Push((void*)&byteCode[pc], type->Size());
				pc += type->Size();
			}
				break;
			case EVT_STRING:
			{
				FString string;
				for (char ch = 0; true; pc++)
				{
					ch = byteCode[pc];
					if (ch == '\0')
						break;

					string += ch;
				}

				//stack.Push(string.data(), string.size() + 1);
				stack.PushString(string);
			}
				break;
			case EVT_CLASS_PTR:
			{
				uint64 ptr = *(uint64*)&byteCode[pc];
				pc += sizeof(uint64);
				stack.Push(&ptr);
			}
				break;
			case EVT_STRUCT_PTR:
			{
				uint64 ptr = *(uint64*)&byteCode[pc];
				pc += sizeof(uint64);
				stack.Push(&ptr);
			}
				break;
			case EVT_ENUM_PTR:
			{
				uint64 ptr = *(uint64*)&byteCode[pc];
				pc += sizeof(uint64);
				stack.Push(&ptr);
			}
				break;
			case EVT_FLOAT:
			{
				float v = *(float*)&byteCode[pc];
				pc += sizeof(float);
				stack.Push(&v);
			}
				break;
			case EVT_DOUBLE:
			{
				double v = *(double*)&byteCode[pc];
				pc += sizeof(double);
				stack.Push(&v);
			}
				break;
			case EVT_INT:
			{
				int32 v = *(int32*)&byteCode[pc];
				pc += sizeof(int32);
				stack.Push(&v);
			}
				break;
			case EVT_INT8:
			{
				int8 v = *(int8*)&byteCode[pc];
				pc += sizeof(int8);
				stack.Push(&v);
			}
				break;
			case EVT_INT16:
			{
				int16 v = *(int16*)&byteCode[pc];
				pc += sizeof(int16);
				stack.Push(&v);
			}
				break;
			case EVT_INT64:
			{
				int64 v = *(int64*)&byteCode[pc];
				pc += sizeof(int64);
				stack.Push(&v);
			}
				break;

			case EVT_UINT:
			{
				uint32 v = *(uint32*)&byteCode[pc];
				pc += sizeof(uint32);
				stack.Push(&v);
			}
				break;
			case EVT_UINT8:
			{
				uint8 v = *(uint8*)&byteCode[pc];
				pc += sizeof(uint8);
				stack.Push(&v);
			}
				break;
			case EVT_UINT16:
			{
				uint16 v = *(uint16*)&byteCode[pc];
				pc += sizeof(uint16);
				stack.Push(&v);
			}
				break;
			case EVT_UINT64:
			{
				uint64 v = *(uint64*)&byteCode[pc];
				pc += sizeof(uint64);
				stack.Push(&v);
			}
				break;

			case EVT_BOOL:
			{
				int8 v = *(int8*)&byteCode[pc];
				pc += sizeof(int8);
				stack.Push(&v);
			}
				break;
			}
			continue;
		}

		if (opCode == EThOpCode::PUSH_VAR_B)
		{
			pc++;
			int varI = *(int*)&byteCode[pc];
			pc += sizeof(int);
			EDataType type;
			type = *(EDataType*)&byteCode[pc];
			pc++;

			FVariable* var = stack.GetVariable(varI);
			if (!var)
			{
				SCRIPT_LogError("OPERATION:PUSH_VAR attempted to get local variable with invalid index!");
				return 1;
			}

			if (var->type != type)
			{
				SCRIPT_LogError("OPERATION:PUSH_VAR target variable has different type!");
				return 1;
			}

			switch (type)
			{
			case EVT_STRUCT:
			{
				FStruct* type = CModuleManager::GetStruct(var->typeId);

				if (!type)
				{
					SCRIPT_LogError("OPERATION:PUSH_VAR target variable has invalid type id (" + FString::ToString(var->typeId) + ")!");
					return 1;
				}

				stack.Push(var->data.Data(), type->Size());
			}
				break;
			case EVT_CLASS:
				break;
			case EVT_STRING:
			{
				FString v = (const char*)var->data.Data();
				stack.PushString(v);
			}
				break;
			case EVT_ENUM:
			{
				FEnum* type = CModuleManager::GetEnum(var->typeId);

				if (!type)
				{
					SCRIPT_LogError("OPERATION:PUSH_VAR target variable has invalid type id (" + FString::ToString(var->typeId) + ")!");
					return 1;
				}

				stack.Push(var->data.Data(), type->Size());
			}
				break;
			case EVT_ARRAY:
				break;
			case EVT_OBJECT_PTR:
				stack.Push(var->data.Data(), 8);
				break;
			case EVT_CLASS_PTR:
				stack.Push(var->data.Data(), 8);
				break;
			case EVT_STRUCT_PTR:
				stack.Push(var->data.Data(), 8);
				break;
			case EVT_ENUM_PTR:
				stack.Push(var->data.Data(), 8);
				break;
			case EVT_FLOAT:
				stack.Push(var->data.Data(), 4);
				break;
			case EVT_DOUBLE:
				stack.Push(var->data.Data(), 8);
				break;
			case EVT_INT:
				stack.Push(var->data.Data(), 4);
				break;
			case EVT_INT8:
				stack.Push(var->data.Data(), 1);
				break;
			case EVT_INT16:
				stack.Push(var->data.Data(), 2);
				break;
			case EVT_INT64:
				stack.Push(var->data.Data(), 8);
				break;
			case EVT_UINT:
				stack.Push(var->data.Data(), 4);
				break;
			case EVT_UINT8:
				stack.Push(var->data.Data(), 1);
				break;
			case EVT_UINT16:
				stack.Push(var->data.Data(), 2);
				break;
			case EVT_UINT64:
				stack.Push(var->data.Data(), 8);
				break;
			case EVT_BOOL:
				stack.Push(var->data.Data(), 1);
				break;
			}

			continue;
		}

		if (opCode == EThOpCode::PUSH_VAR_C)
		{
			SizeType targetId{};
			stack.Pop(&targetId);

			pc++;
			//uint numArgs = *(uint*)&byteCode[pc];
			//pc += sizeof(uint);

			if (targetId == 0)
			{
				SCRIPT_LogError("NULL PTR EXCEPTION! attempted to retrieve variable from null ptr");
				return 1;
			}

			CObject* target = nullptr;
			if (targetId == THSCRIPT_THIS)
				target = obj;
			else
				target = CObjectManager::FindObject(targetId);

			if (!target)
			{
				SCRIPT_LogError("OPERATION:PUSH_VAR target does not exists! (" + FString::ToString(targetId) + ")");
				return 1;
			}

			const FProperty* curProp = nullptr;
			EDataType type = *(EDataType*)&byteCode[pc];
			pc++;

			uint64 propId = *(uint64*)&byteCode[pc];
			pc += sizeof(uint64);

			//uint32 offset = *(uint32*)&byteCode[pc];
			//pc += sizeof(uint32);

			//uint32 size = *(uint32*)&byteCode[pc];
			//pc += sizeof(uint32);

			curProp = target->GetClass()->GetProperty(propId);
			if (!curProp)
			{
				SCRIPT_LogError("OPERATION:PUSH_VAR invalid property id! (" + FString::ToString(propId) + ")");
				return 1;
			}

			if (curProp->type != type)
				return 1;

			// push the data
			uint8* data = ((uint8*)target) + curProp->offset;

			if (type == EVT_STRING)
			{
				FString& str = *(FString*)data;
				stack.PushString(str);
			}
			else if (type == EVT_OBJECT_PTR)
			{
				TObjectPtr<CObject>& ptr = *(TObjectPtr<CObject>*)data;
				SizeType id = 0;
				if (ptr.IsValid())
					id = ptr->Id();
				
				stack.Push(&id);
			}
			else if (type == EVT_ARRAY);
			else
				stack.Push(data, curProp->size);
		}

		if (opCode == EThOpCode::PUSH_VAR_F)
		{
			pc++;
			int reg = *(int*)&byteCode[pc];
			pc += sizeof(int);

			EDataType type = *(EDataType*)&byteCode[pc];
			pc++;

			if (type == EVT_STRING || type == EVT_STRUCT || type == EVT_ARRAY)
			{
				SCRIPT_LogError("OPERATION:PUSH_VAR invalid type " + VariableTypeToString(type) + "! registers can only hold 64bit data types");
				return 1;
			}

			switch (type)
			{
			case EVT_OBJECT_PTR:
				stack.Push(&stack.registers[reg], 8);
				break;
			case EVT_CLASS_PTR:
				stack.Push(&stack.registers[reg], 8);
				break;
			case EVT_STRUCT_PTR:
				stack.Push(&stack.registers[reg], 8);
				break;
			case EVT_ENUM_PTR:
				stack.Push(&stack.registers[reg], 8);
				break;
			case EVT_FLOAT:
				stack.Push(&stack.registers[reg], 4);
				break;
			case EVT_DOUBLE:
				stack.Push(&stack.registers[reg], 8);
				break;
			case EVT_INT:
				stack.Push(&stack.registers[reg], 4);
				break;
			case EVT_INT8:
				stack.Push(&stack.registers[reg], 1);
				break;
			case EVT_INT16:
				stack.Push(&stack.registers[reg], 2);
				break;
			case EVT_INT64:
				stack.Push(&stack.registers[reg], 8);
				break;
			case EVT_UINT:
				stack.Push(&stack.registers[reg], 4);
				break;
			case EVT_UINT8:
				stack.Push(&stack.registers[reg], 1);
				break;
			case EVT_UINT16:
				stack.Push(&stack.registers[reg], 2);
				break;
			case EVT_UINT64:
				stack.Push(&stack.registers[reg], 8);
				break;
			case EVT_BOOL:
				stack.Push(&stack.registers[reg], 1);
				break;
			}
		}

		if (opCode == EThOpCode::PUSH_VAR_G)
		{

		}

		if (opCode == EThOpCode::MOVE_A)
		{
			pc++;
			uint8 destRegister = byteCode[pc];

			pc++;
			int64 value = *(int64*)&byteCode[pc];

			pc += sizeof(int64);

			stack.registers[destRegister] = value;
		}

		if (opCode == EThOpCode::MOVE_B)
		{
			pc++;
			int destRegister = byteCode[pc];

			
		}

		if (opCode == EThOpCode::CALL)
		{
			pc++;
			uint64 typeId = *(uint64*)&byteCode[pc];
			pc += sizeof(uint64);
			uint64 funcId = *(uint64*)&byteCode[pc];
			pc += sizeof(uint64);

			FClass* type = CModuleManager::GetClass(typeId);
			if (!type)
			{
				SCRIPT_LogError("OPERATION:CALL had invalid type id! (" + FString::ToString(typeId) + ")");
				return 1;
			}

			const FFunction* tfunc = type->GetFunction(funcId);
			if (!tfunc)
			{
				SCRIPT_LogError("OPERATION:CALL had invalid function id! (" + FString::ToString(funcId) + ")");
				return 1;
			}

			CObject* target = nullptr;
			if (tfunc->flags & FunctionFlags_STATIC == 0)
			{
				size_t targetId;
				stack.Pop(&targetId, sizeof(size_t));

				if (targetId == THSCRIPT_THIS)
					target = obj;
				else
					target = CObjectManager::FindObject(targetId);

				if (!target)
				{
					SCRIPT_LogError("EXCEPTION! attempted to call function '" + tfunc->cppName + "' on invalid object (" + FString::ToString(targetId) + ")!");
					return 1;
				}
			}

			if (!tfunc->execFunc(target, stack))
				return 1;
			//pc++;
		}
	}

	stack.PopStackIndex();
	return 0;
}

void CThsClass::AddFunction(FScriptFunction* func)
{

}

void CThsClass::AddProperty(const FThsProperty& p)
{

}
