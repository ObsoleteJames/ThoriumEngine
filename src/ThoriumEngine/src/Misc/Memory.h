#pragma once

#include "EngineCore.h"
#include <map>

#if IS_DEV
//#undef malloc
//#undef free
//#undef realloc
//#undef _expand

//#define malloc(s) gMemory.Alloc(s, __FILE__, __LINE__)
//#define free(p) gMemory.Free(p, __FILE__, __LINE__)
//#define realloc(p, s) gMemory.Realloc(p, s, __FILE__, __LINE__)
//#define _expand(p, s) gMemory.Expand(p, s, __FILE__, __LINE__)

class ENGINE_API CMemory
{
public:
	inline void* Alloc(size_t size) { return Alloc(size, "unkown", 0); }
	inline void Free(void* ptr) { Free(ptr, "unknown", 0); }
	inline void Expand(void* ptr, size_t size) { Expand(ptr, size, "unkown", 0); }
	inline void* Realloc(void* ptr, size_t size) { return Realloc(ptr, size, "unkown", 0); }

	void* Alloc(size_t size, const char* file, int line);
	void Free(void* ptr, const char* file, int line);
	void* Expand(void* ptr, size_t size, const char* file, int line);
	void* Realloc(void* ptr, size_t size, const char* file, int line);

private:
	struct FMemInfo
	{
		SizeType currentSize;
		SizeType peakSize;
		
		SizeType currentCount;
		SizeType peakCount;
	};

	struct FMemInfoKey
	{
		const char* file;
		int line;
		bool operator<(const FMemInfoKey& key) const;
	};

	struct FMemPtrInfo
	{
		const char* allocFile;
		int allocLine;

		SizeType size;
	};

	FMemInfo& FindOrCreateEntry(const char* file, int line);

	void RegisterAllocation(const char* file, int line, void* ptr, SizeType size);
	void RegisterDeallocation(const char* file, int line, void* ptr);

private:
	FMemInfo globalInfo;
	std::map<FMemInfoKey, FMemInfo> memInfo;
	std::map<void*, FMemPtrInfo> ptrInfo;

};

extern ENGINE_API CMemory gMemory;

#endif
