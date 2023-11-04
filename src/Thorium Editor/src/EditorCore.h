#pragma once

enum EItemTypes
{
	EItemTypes_Folder = 1000,
	EItemTypes_ModFolder,
	EItemTypes_GenericFile,
	EItemTypes_AssetFile,
	EItemTypes_Entity,
	EItemTypes_EntityComponent,
	EItemTypes_SceneComponent
};

#ifdef SDK_DLL
#define SDK_API __declspec(dllexport)
#else
#define SDK_API __declspec(dllimport)
#endif
