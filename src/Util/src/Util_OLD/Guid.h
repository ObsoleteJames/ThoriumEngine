#pragma once

#include <Util/Core.h>

struct FGuid
{
public:
	FGuid();
	inline FGuid(uint64 id) { this->id = id; }

	inline bool operator==(const FGuid& right) const { return right.id == id; }
	inline bool operator==(uint64 right) const { return id == right; }
	inline FGuid& operator=(uint64 right) { id = right; return *this; }
	inline operator uint64() const { return id; }


	inline bool operator>(const FGuid& right) const { return id > right.id; }
	inline bool operator<(const FGuid& right) const { return id < right.id; }

	inline bool operator>=(const FGuid& right) const { return id >= right.id; }
	inline bool operator<=(const FGuid& right) const { return id <= right.id; }

private:
	uint64 id;
};
