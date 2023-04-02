
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Object/ObjectHandle.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

DECLARE_PROPERTY(FObjectHandle, "Object Id", objectId, "", "SizeType", EVT_UINT, VTAG_SERIALIZABLE , offsetof(FObjectHandle, objectId), sizeof(SizeType), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(FObjectHandle, objectId)

class FStruct_FObjectHandle : public FStruct
{
public:
	FStruct_FObjectHandle()
	{
		name = "Object Handle";
		cppName = "FObjectHandle";
		size = sizeof(FObjectHandle);
		numProperties = 1;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = false;
		GetModule_Engine().RegisterFStruct(this);
	}
};
FStruct_FObjectHandle __FStruct_FObjectHandle_Instance;

FStruct* FObjectHandle::StaticStruct() { return &__FStruct_FObjectHandle_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
