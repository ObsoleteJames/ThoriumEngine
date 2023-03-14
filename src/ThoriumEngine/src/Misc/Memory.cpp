
#include "Memory.h"

CMemory gMemory;

void* CMemory::Alloc(size_t size, const char* file, int line)
{
	void* mem = malloc(size);

	RegisterAllocation(file, line, mem, size);

	return mem;
}

void CMemory::Free(void* ptr, const char* file, int line)
{
	if (!ptr)
		return;

	RegisterDeallocation(file, line, ptr);

	free(ptr);
}

void* CMemory::Expand(void* ptr, size_t size, const char* file, int line)
{
	if (!ptr)
		return;

	FMemPtrInfo pf = ptrInfo[ptr];
	RegisterDeallocation(file, line, ptr);

	ptr = _expand(ptr, size);

	RegisterAllocation(pf.allocFile, pf.allocLine, ptr, size);
	return ptr;
}

void* CMemory::Realloc(void* ptr, size_t size, const char* file, int line)
{
	if (!ptr)
		return;

	FMemPtrInfo pf = ptrInfo[ptr];
	RegisterDeallocation(file, line, ptr);

	ptr = realloc(ptr, size);

	RegisterAllocation(pf.allocFile, pf.allocLine, ptr, size);
	return ptr;
}

CMemory::FMemInfo& CMemory::FindOrCreateEntry(const char* file, int line)
{
	auto it = memInfo.find({ file, line });
	if (it != memInfo.end())
		return it->second;

	FMemInfo& info = memInfo[{file, line}];
	return info;
}

void CMemory::RegisterAllocation(const char* file, int line, void* ptr, SizeType size)
{
	FMemInfo& info = FindOrCreateEntry(file, line);

	info.currentCount++;
	info.currentSize += size;

	if (info.currentSize > info.peakSize)
		info.peakSize = info.currentSize;
	if (info.currentCount > info.peakCount)
		info.peakCount = info.currentCount;

	globalInfo.currentCount++;
	globalInfo.currentSize += size;

	if (globalInfo.currentSize > globalInfo.peakSize)
		globalInfo.peakSize = globalInfo.currentSize;
	if (globalInfo.currentCount > globalInfo.peakCount)
		globalInfo.peakCount = globalInfo.currentCount;

	ptrInfo[ptr] = { file, line, size };
}

void CMemory::RegisterDeallocation(const char* file, int line, void* ptr)
{
	FMemPtrInfo& pf = ptrInfo[ptr];
	FMemInfo& info = FindOrCreateEntry(pf.allocFile, pf.allocLine);

	info.currentCount--;
	info.currentSize -= pf.size;

	ptrInfo.erase(ptr);
}

bool CMemory::FMemInfoKey::operator<(const FMemInfoKey& key) const
{
	int r = stricmp(file, key.file);
	if (r < 0)
		return true;

	if (r > 0)
		return false;

	return line < key.line;
}

void* operator new(std::size_t size)
{
	return gMemory.Alloc(size);
}

void* operator new[](std::size_t size)
{
	return gMemory.Alloc(size);
}

void operator delete(void* ptr)
{
	gMemory.Free(ptr);
}

void operator delete[](void* ptr)
{
	gMemory.Free(ptr);
}
