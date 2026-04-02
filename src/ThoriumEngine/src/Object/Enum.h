#pragma once

#include <Util/Types.h>

/**
*  helper class for defining enums that should be stored as ints.
*  Useful for byte flags, where the enum values are defined as bitmasks.
* 
*  T is the enum type.
*  T2 is the underlying type of the enum, defaulting to int.
*/
template<typename T, typename T2 = uint8>
class TEnumInt
{
public:
	typedef T Type;

public:
	constexpr TEnumInt() : value(0) {}
	constexpr TEnumInt(const TEnumInt& other) : value(other.value) {}
	constexpr TEnumInt(T enumValue) : value(static_cast<T2>(enumValue)) {}
	constexpr TEnumInt(T2 intValue) : value(intValue) {}

	// cast operators
	operator T2() const { return value; }
	operator T() const { return static_cast<T>(value); }

	TEnumInt& operator=(const TEnumInt& other) { value = other.value; return *this; }
	TEnumInt& operator=(T enumValue) { value = static_cast<T2>(enumValue); return *this; }
	TEnumInt& operator=(T2 intValue) { value = intValue; return *this; }

	T2 operator|(T2 other) const { return value | other; }
	T2 operator&(T2 other) const { return value & other; }
	T2 operator^(T2 other) const { return value ^ other; }
	T2 operator+(T2 other) const { return value + other; }
	T2 operator-(T2 other) const { return value - other; }
	T2 operator*(T2 other) const { return value * other; }
	T2 operator/(T2 other) const { return value / other; }
	
	T2 operator~() const { return ~value; }

	TEnumInt& operator|=(T2 other) { value |= other; return *this; }
	TEnumInt& operator&=(T2 other) { value &= other; return *this; }
	TEnumInt& operator^=(T2 other) { value ^= other; return *this; }
	TEnumInt& operator+=(T2 other) { value += other; return *this; }
	TEnumInt& operator-=(T2 other) { value -= other; return *this; }
	TEnumInt& operator*=(T2 other) { value *= other; return *this; }
	TEnumInt& operator/=(T2 other) { value /= other; return *this; }

	bool operator==(const TEnumInt& other) const { return value == other.value; }
	bool operator!=(const TEnumInt& other) const { return value != other.value; }
	bool operator==(T enumValue) const { return value == static_cast<T2>(enumValue); }
	bool operator!=(T enumValue) const { return value != static_cast<T2>(enumValue); }
	bool operator==(T2 intValue) const { return value == intValue; }
	bool operator!=(T2 intValue) const { return value != intValue; }

public:
	T2 value;
};
