#pragma once

enum EVariableType
{
	EVT_NULL,
	EVT_VOID,
	EVT_STRUCT,
	EVT_CLASS,
	EVT_STRING,
	EVT_ENUM,
	EVT_ARRAY,
	EVT_OBJECT_PTR,

	EVT_CLASS_PTR,
	EVT_STRUCT_PTR,
	EVT_ENUM_PTR,

	// Primitive types
	EVT_FLOAT,
	EVT_DOUBLE,
	EVT_INT,
	EVT_UINT,
	EVT_BOOL,

	EVT_END // Used to mark the end of enum.
};

enum EVariableFlags
{
	VTAG_NONE = 0,

	VTAG_TYPE_POINTER = 1,
	VTAG_ARRAY_FIRST_CLASS = 1 << 1, // Used to check if the element in array/map is a class.
	VTAG_ARRAY_SECOND_CLASS = 1 << 2, // Used to check if the second element in a map is a class.

	VTAG_EDITOR_VISIBLE = 1 << 3,
	VTAG_EDITOR_EDITABLE = 1 << 4,
	VTAG_SERIALIZABLE = 1 << 5,
	VTAG_STATIC = 1 << 6,
};

enum EClassFlags
{
	CTAG_NONE = 0,

	// Class can't be Instantiated.
	CTAG_ABSTRACT = 1,
	// Class is hidden in Editor
	CTAG_HIDDEN = 1 << 1,
	// Class only contains static functions and variables
	CTAG_STATIC = 1 << 2,

};

enum EAssetFlags
{
	ASSET_NONE = 0,

	ASSET_AUTO_LOAD = 1,
	ASSET_COMPILABLE = 1 << 1,
};
