
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Rendering/Buffers.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

#ifdef IS_DEV
static TPair<FString, FString> _FClass_IVertexBuffer_Tags[] {
	{ "Abstract", "" },
};
#endif

class FClass_IVertexBuffer : public FClass
{
public:
	FClass_IVertexBuffer()
	{
		name = "Vertex Buffer";
		cppName = "IVertexBuffer";
		size = sizeof(IVertexBuffer);
		numProperties = 0;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
#ifdef IS_DEV
		numTags = 1;
		tags = _FClass_IVertexBuffer_Tags;
#endif
		BaseClass = CObject::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_ABSTRACT;
		GetModule_Engine().RegisterFClass(this);
	}
	CObject* Instantiate() override { return nullptr; }
};
FClass_IVertexBuffer __FClass_IVertexBuffer_Instance;

FClass* IVertexBuffer::StaticClass() { return &__FClass_IVertexBuffer_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

#ifdef IS_DEV
static TPair<FString, FString> _FClass_IIndexBuffer_Tags[] {
	{ "Abstract", "" },
};
#endif

class FClass_IIndexBuffer : public FClass
{
public:
	FClass_IIndexBuffer()
	{
		name = "Index Buffer";
		cppName = "IIndexBuffer";
		size = sizeof(IIndexBuffer);
		numProperties = 0;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
#ifdef IS_DEV
		numTags = 1;
		tags = _FClass_IIndexBuffer_Tags;
#endif
		BaseClass = CObject::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_ABSTRACT;
		GetModule_Engine().RegisterFClass(this);
	}
	CObject* Instantiate() override { return nullptr; }
};
FClass_IIndexBuffer __FClass_IIndexBuffer_Instance;

FClass* IIndexBuffer::StaticClass() { return &__FClass_IIndexBuffer_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

#ifdef IS_DEV
static TPair<FString, FString> _FClass_IShaderBuffer_Tags[] {
	{ "Abstract", "" },
};
#endif

class FClass_IShaderBuffer : public FClass
{
public:
	FClass_IShaderBuffer()
	{
		name = "Shader Buffer";
		cppName = "IShaderBuffer";
		size = sizeof(IShaderBuffer);
		numProperties = 0;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
#ifdef IS_DEV
		numTags = 1;
		tags = _FClass_IShaderBuffer_Tags;
#endif
		BaseClass = CObject::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_ABSTRACT;
		GetModule_Engine().RegisterFClass(this);
	}
	CObject* Instantiate() override { return nullptr; }
};
FClass_IShaderBuffer __FClass_IShaderBuffer_Instance;

FClass* IShaderBuffer::StaticClass() { return &__FClass_IShaderBuffer_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
