
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Math/Vectors.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

DECLARE_PROPERTY(FVector, "x", x, "", "float", EVT_FLOAT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(FVector, x), sizeof(float), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(FVector, x)

DECLARE_PROPERTY(FVector, "y", y, "", "float", EVT_FLOAT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(FVector, y), sizeof(float), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(FVector, y)

DECLARE_PROPERTY(FVector, "z", z, "", "float", EVT_FLOAT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(FVector, z), sizeof(float), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(FVector, z)

class FStruct_FVector : public FStruct
{
public:
	FStruct_FVector()
	{
		name = "Vector";
		cppName = "FVector";
		size = sizeof(FVector);
		numProperties = 3;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = false;
		GetModule_Engine().RegisterFStruct(this);
	}
};
FStruct_FVector __FStruct_FVector_Instance;

FStruct* FVector::StaticStruct() { return &__FStruct_FVector_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

DECLARE_PROPERTY(FQuaternion, "x", x, "", "float", EVT_FLOAT, VTAG_SERIALIZABLE , offsetof(FQuaternion, x), sizeof(float), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(FQuaternion, x)

DECLARE_PROPERTY(FQuaternion, "y", y, "", "float", EVT_FLOAT, VTAG_SERIALIZABLE , offsetof(FQuaternion, y), sizeof(float), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(FQuaternion, y)

DECLARE_PROPERTY(FQuaternion, "z", z, "", "float", EVT_FLOAT, VTAG_SERIALIZABLE , offsetof(FQuaternion, z), sizeof(float), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(FQuaternion, z)

DECLARE_PROPERTY(FQuaternion, "w", w, "", "float", EVT_FLOAT, VTAG_SERIALIZABLE , offsetof(FQuaternion, w), sizeof(float), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(FQuaternion, w)

class FStruct_FQuaternion : public FStruct
{
public:
	FStruct_FQuaternion()
	{
		name = "Quaternion";
		cppName = "FQuaternion";
		size = sizeof(FQuaternion);
		numProperties = 4;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = false;
		GetModule_Engine().RegisterFStruct(this);
	}
};
FStruct_FQuaternion __FStruct_FQuaternion_Instance;

FStruct* FQuaternion::StaticStruct() { return &__FStruct_FQuaternion_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

DECLARE_PROPERTY(FRay, "Origin", origin, "", "FVector", EVT_STRUCT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(FRay, origin), sizeof(FVector), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(FRay, origin)

DECLARE_PROPERTY(FRay, "Direction", direction, "", "FVector", EVT_STRUCT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(FRay, direction), sizeof(FVector), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(FRay, direction)

class FStruct_FRay : public FStruct
{
public:
	FStruct_FRay()
	{
		name = "Ray";
		cppName = "FRay";
		size = sizeof(FRay);
		numProperties = 2;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = false;
		GetModule_Engine().RegisterFStruct(this);
	}
};
FStruct_FRay __FStruct_FRay_Instance;

FStruct* FRay::StaticStruct() { return &__FStruct_FRay_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

class FStruct_FMatrix : public FStruct
{
public:
	FStruct_FMatrix()
	{
		name = "Matrix";
		cppName = "FMatrix";
		size = sizeof(FMatrix);
		numProperties = 0;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = false;
		GetModule_Engine().RegisterFStruct(this);
	}
};
FStruct_FMatrix __FStruct_FMatrix_Instance;

FStruct* FMatrix::StaticStruct() { return &__FStruct_FMatrix_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
