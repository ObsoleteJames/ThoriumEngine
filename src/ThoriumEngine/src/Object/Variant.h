#pragma once

#include "Math/Vectors.h"
#include "Math/Color.h"
#include <Util/Core.h>

class ENGINE_API FVariant
{
public:
	enum EType
	{
		INVALID,
		INTEGER,
		BOOL,
		FLOAT,
		DOUBLE,
		SIZETYPE,
		OBJ_POINTER,

		VECTOR,
		COLOR,
		STRING,
		CLASS
	};

	FVariant(const FVariant&);
	~FVariant();

	explicit FVariant(int v);
	explicit FVariant(bool v);
	explicit FVariant(float v);
	explicit FVariant(double v);
	explicit FVariant(SizeType v);
	explicit FVariant(CObject* v);

	explicit FVariant(const FVector& v);
	explicit FVariant(const FColor& v);
	explicit FVariant(const FString& v);
	explicit FVariant(const char* v);
	explicit FVariant(FClass* v);

	int AsInt() const;
	bool AsBool() const;
	float AsFloat() const;
	double AsDouble() const;
	SizeType AsSizeType() const;
	CObject* AsObjectPointer() const;

	FVector AsVector() const;
	FColor AsColor() const;
	FString AsString() const;
	FClass* AsClass() const;

	inline EType Type() const { return type; }

	FString ToString() const;
	static FVariant FromString(const FString&);

public:
	FVariant& operator=(const FVariant&);

	bool operator==(const FVariant&) const;
	inline bool operator!=(const FVariant& other) const { return !(*this == other); }

	template<typename T>
	FVariant& operator=(const T& other)
	{
		*this = FVariant(other);
		return *this;
	}

private:
	union {
		// Primitives
		int uInteger;
		bool uBool;
		float uFloat;
		double uDouble;
		SizeType uSizeType;
		SizeType uObjPointer;

		// Objects
		FVector uVector;
		FColor uColor;
		FString uString;
		FClass* uClass;
	};

	EType type;
};
