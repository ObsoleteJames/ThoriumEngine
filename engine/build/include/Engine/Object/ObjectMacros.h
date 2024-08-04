#pragma once

#include "Class.h"

#if __GNUC__
#	pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif

#define STRUCT(...)
#define CLASS(...)
#define ENUM(...)
#define PROPERTY(...)
#define FUNCTION(...)
#define META(...)
#define ASSET(...)

#define _BODY_MACRO_COMBINE_INNER(A, B, C, D) A##B##C##D
#define _BODY_MACRO_COMBINE(A, B, C, D) _BODY_MACRO_COMBINE_INNER(A, B, C, D)

#define GENERATED_BODY() _BODY_MACRO_COMBINE(FILE_ID, _, __LINE__, _GeneratedBody)

#define DECLARE_FUNCTION(func, ...) static void func(__VA_ARGS__);
#define DEFINE_FUNCTION(func, ...) void func(__VA_ARGS__);

#define DECLARE_EXEC_FUNCTION(func) public: DECLARE_FUNCTION(exec##func, CObject* target, FStack& stack)
#define DECLARE_IMPLEMENTATION(func, ...) void func##_Implementation(__VA_ARGS__);


// Reflection Helpers
#define PRIVATE_MEMBER_OFFSET_ACCESSOR(TClass, memberName) public: \
static inline SizeType __private_##memberName##_offset() { return offsetof(TClass, memberName); }\

#define DECLARE_CLASS(TClass, TBaseClass, TModuleName) \
public: \
	typedef TBaseClass BaseClass; \
	static FClass* StaticClass(); \
	static inline FString StaticModule() { return #TModuleName; } \
	virtual FClass* GetClass() const override { return TClass::StaticClass(); } \
	virtual FString GetModule() const override { return TClass::StaticModule(); } \
private:

#define DECLARE_STRUCT(TStruct, TModuleName) \
public: \
	static FStruct* StaticStruct(); \
	static inline FString StaticModule() { return #TModuleName; } \

#define EVALUATE_PROPERTY_NAME(TClass, InternalName) TClass##_##InternalName##_Property

#if defined(INCLUDE_EDITOR_DATA)
#define DECLARE_PROPERTY(TClass, DisplayName, InternalName, Description, TypeName, TType, TTags, TOffset, TSize, TMetaData, TTypeHelper, TId, TProtection) \
static FProperty  EVALUATE_PROPERTY_NAME(TClass, InternalName) { DisplayName, #InternalName, Description, TId, TProtection, TypeName, TType, TTags, TOffset, TSize, TMetaData, TTypeHelper, CLASS_NEXT_PROPERTY };
#else
#define DECLARE_PROPERTY(TClass, DisplayName, InternalName, Description, TypeName, TType, TTags, TOffset, TSize, TMetaData, TTypeHelper, TId, TProtection) \
static FProperty  EVALUATE_PROPERTY_NAME(TClass, InternalName) { DisplayName, #InternalName, TId, TProtection, TypeName, TType, TTags, TOffset, TSize, TMetaData, TTypeHelper, CLASS_NEXT_PROPERTY };
#endif

#define EVALUATE_FUNCTION_NAME(TClass, InternalName) TClass##_##InternalName##_Function

#if defined(INCLUDE_EDITOR_DATA)
#define DECLARE_FUNCTION_PROPERTY(TClass, DisplayName, Description, InternalName, TFuncPtr, TType, TArgList, TArgCount, flags, TId, TProtection, TReturnType, TReturnTypeID) \
static FFunction EVALUATE_FUNCTION_NAME(TClass, InternalName) { DisplayName, #InternalName, Description, TId, TProtection, TFuncPtr, TType, { TReturnType, TReturnTypeID, EVT_NULL, 0, 0, 0, 0 }, TArgCount, TArgList, flags, CLASS_NEXT_FUNCTION };
#else
#define DECLARE_FUNCTION_PROPERTY(TClass, DisplayName, Description, InternalName, TFuncPtr, TType, TArgList, TArgCount, flags, TId, TProtection, TReturn) \
static FFunction EVALUATE_FUNCTION_NAME(TClass, InternalName) { DisplayName, #InternalName, TId, TProtection, TFuncPtr, TType, TReturn, TArgCount, TArgList, flags, CLASS_NEXT_FUNCTION };
#endif

#define POP_STACK_VARIABLE(Type, OutVar) Type OutVar; stack.Pop(OutVar)
