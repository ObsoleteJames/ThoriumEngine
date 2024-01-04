
#include "Util/MemStream.h"
#include <cstring>
#include <algorithm>

void FMemStream::Read(void* buffer, SizeType size)
{
	if (EndOfFile())
		return;

	SizeType toRead = std::min(size, Size() - cursor);

	memcpy(buffer, Data() + cursor, toRead);
	cursor += toRead;
}

void FMemStream::Write(void* buffer, SizeType size)
{
	if (cursor + size > Size())
		this->Resize(Size() + size);
	memcpy(Data() + cursor, buffer, size);
	cursor += size;
}

void FMemStream::Seek(SizeType to, uint8_t from)
{
	switch (from)
	{
	case SEEK_SET:
		cursor = std::min(Size(), to);
		break;
	case SEEK_CUR:
		cursor = std::min(Size(), cursor + to);
		break;
	case SEEK_END:
		cursor = Size();
		break;
	}
}
