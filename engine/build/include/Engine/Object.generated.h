
#include "Object/Class.h"
#include "Object/ObjectMacros.h"

#define Engine_Object_h_114_GeneratedBody \
PRIVATE_MEMBER_OFFSET_ACCESSOR(CObject, name) \
PRIVATE_MEMBER_OFFSET_ACCESSOR(CObject, Owner) \
PRIVATE_MEMBER_OFFSET_ACCESSOR(CObject, Children) \
DECLARE_EXEC_FUNCTION(OnNetDelete)\
DECLARE_IMPLEMENTATION(OnNetDelete)\
DECLARE_EXEC_FUNCTION(OnOwnerChanged)\
DECLARE_IMPLEMENTATION(OnOwnerChanged, SizeType ownerId)\
DECLARE_CLASS(CObject, CObjectBase, Engine)


#undef FILE_ID
#define FILE_ID Engine_Object_h
