
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Math/Vectors.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

DECLARE_PROPERTY(FVector2, "x", x, "", "float", EVT_FLOAT, VTAG_SERIALIZABLE , offsetof(FVector2, x), sizeof(float), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(FVector2, x)

DECLARE_PROPERTY(FVector2, "y", y, "", "float", EVT_FLOAT, VTAG_SERIALIZABLE , offsetof(FVector2, y), sizeof(float), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(FVector2, y)

class FStruct_FVector2 : public FStruct
{
public:
	FStruct_FVector2()
	{
		name = "Vector 2";
		cppName = "FVector2";
		size = sizeof(FVector2);
		numProperties = 2;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = false;
		GetModule_Engine().RegisterFStruct(this);
	}
};
FStruct_FVector2 __FStruct_FVector2_Instance;

FStruct* FVector2::StaticStruct() { return &__FStruct_FVector2_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

#if IS_DEV
static TPair<FString, FString> _FVector_x_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _FVector_x_Meta {
	"",
	"",
	"",
	"",
	1,
	_FVector_x_Meta_Tags
};

#define _FVector_x_Meta_Ptr &_FVector_x_Meta
#else
#define _FVector_x_Meta_Ptr nullptr
#endif
DECLARE_PROPERTY(FVector, "x", x, "", "float", EVT_FLOAT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(FVector, x), sizeof(float), _FVector_x_Meta_Ptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(FVector, x)

#if IS_DEV
static TPair<FString, FString> _FVector_y_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _FVector_y_Meta {
	"",
	"",
	"",
	"",
	1,
	_FVector_y_Meta_Tags
};

#define _FVector_y_Meta_Ptr &_FVector_y_Meta
#else
#define _FVector_y_Meta_Ptr nullptr
#endif
DECLARE_PROPERTY(FVector, "y", y, "", "float", EVT_FLOAT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(FVector, y), sizeof(float), _FVector_y_Meta_Ptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(FVector, y)

#if IS_DEV
static TPair<FString, FString> _FVector_z_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _FVector_z_Meta {
	"",
	"",
	"",
	"",
	1,
	_FVector_z_Meta_Tags
};

#define _FVector_z_Meta_Ptr &_FVector_z_Meta
#else
#define _FVector_z_Meta_Ptr nullptr
#endif
DECLARE_PROPERTY(FVector, "z", z, "", "float", EVT_FLOAT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(FVector, z), sizeof(float), _FVector_z_Meta_Ptr, nullptr)
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

#if IS_DEV
static TPair<FString, FString> _FRay_origin_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _FRay_origin_Meta {
	"",
	"",
	"",
	"",
	1,
	_FRay_origin_Meta_Tags
};

#define _FRay_origin_Meta_Ptr &_FRay_origin_Meta
#else
#define _FRay_origin_Meta_Ptr nullptr
#endif
DECLARE_PROPERTY(FRay, "Origin", origin, "", "FVector", EVT_STRUCT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(FRay, origin), sizeof(FVector), _FRay_origin_Meta_Ptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(FRay, origin)

#if IS_DEV
static TPair<FString, FString> _FRay_direction_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _FRay_direction_Meta {
	"",
	"",
	"",
	"",
	1,
	_FRay_direction_Meta_Tags
};

#define _FRay_direction_Meta_Ptr &_FRay_direction_Meta
#else
#define _FRay_direction_Meta_Ptr nullptr
#endif
DECLARE_PROPERTY(FRay, "Direction", direction, "", "FVector", EVT_STRUCT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(FRay, direction), sizeof(FVector), _FRay_direction_Meta_Ptr, nullptr)
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
