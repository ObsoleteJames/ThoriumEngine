
#include "ObjectHandle.h"

FObjectHandle::FObjectHandle(CObject* ptr)
{
	if (ptr)
		objectId = ptr->Id();
}

FObjectHandle::FObjectHandle(const FObjectHandle& other)
{
	objectId = other.objectId;
}

FObjectHandle& FObjectHandle::operator=(CObject* obj)
{
	if (obj)
		objectId = obj->Id();
	return *this;
}
