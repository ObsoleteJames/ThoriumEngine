#pragma once

enum EVariableType_
{
	EVT_STRUCT,
	EVT_CLASS, // e.g. CObject*, CEntity*.
	EVT_STRING,
	EVT_ENUM,
	EVT_ARRAY,
	EVT_MAP,
	EVT_OBJECT_PTR, // Same as EVT_CLASS, but using the TObjectPtr container.

	EVT_CLASS_PTR,

	// Primitive types
	EVT_FLOAT,
	EVT_DOUBLE,
	EVT_INT,
	EVT_UINT,
	EVT_BOOL,
	EVT_VOID,

	EVT_END // Used to mark the end of enum.
};
typedef uint8 EVariableType;

enum EVariableFlags_
{
	VTAG_NONE = 0,

	VTAG_TYPE_POINTER = 1,

	// Obsolete flags, replaced by templateType in FProperty.
	VTAG_ARRAY_FIRST_CLASS = 1 << 1, // Used to check if the element in array/map is a class.
	VTAG_ARRAY_SECOND_CLASS = 1 << 2, // Used to check if the second element in a map is a class.

	VTAG_EDITOR_VISIBLE = 1 << 3,
	VTAG_EDITOR_EDITABLE = 1 << 4,
	VTAG_SERIALIZABLE = 1 << 5,
	VTAG_STATIC = 1 << 6,
	VTAG_SAVEGAME = 1 << 7, // Whether this variable should be saved in save game or not.
	VTAG_IGNORE_DEFAULT = 1 << 14, // Wether this variable should always be serialized, even if it's the same as the default value.

	// Network flags
	VTAG_REPLICATED = 1 << 8, // Whether this variable should be replicated to clients or not.
	VTAG_VALIDATE = 1 << 9, // Whether this variable should be validated when updated.
	VTAG_AUTH_SERVER = 1 << 10, // Whether this variable can only be changed by the server.
	VTAG_AUTH_OWNER = 1 << 11, // Whether this variable can only be changed by the owning player.
	VTAG_AUTH_ANY = 1 << 12, // Whether this variable can be changed by anyone.
	VTAG_RELEVANT_TO_OWNER = 1 << 13, // Whether this variable is only relevant to the owning player. other clients won't receive updates about this variable if they don't own this object.
};
typedef uint EVariableFlags;

enum EFunctionFlags_
{
	FTAG_NONE = 0,
	FTAG_ALLOW_AS_INPUT = 1 << 0, // can this function be called from entity IO.
	FTAG_STATIC = 1 << 1,
	FTAG_SCRIPT_VIRTUAL = 1 << 2, // native function that can be overriden in scripts.
	FTAG_SCRIPT_CALLABLE = 1 << 3, // native function that can be called from scripts.
};
typedef int EFunctionFlags;

enum EClassFlags_
{
	CTAG_NONE = 0,

	// Type can't be Instantiated.
	CTAG_ABSTRACT = 1,
	// Type is hidden in Editor
	CTAG_HIDDEN = 1 << 1,
	// Type only contains static functions and variables, only used for script types.
	CTAG_STATIC = 1 << 2,
	// Type is defined in C++.
	CTAG_NATIVE = 1 << 3,
	// Type is a class.
	CTAG_CLASS = 1 << 4,
	// This class is a prefab
	CTAG_PREFAB = 1 << 5, 
};
typedef uint EClassFlags;

enum EAssetFlags_
{
	ASSET_NONE = 0,

	ASSET_AUTO_LOAD = 1, // this asset will be automatically loaded at startup.
	ASSET_COMPILABLE = 1 << 1,
};
typedef uint EAssetFlags;
