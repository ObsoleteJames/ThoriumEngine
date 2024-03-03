
#include <string>
#include "Engine.h"
#include "ModelAsset.h"
#include "Rendering/Renderer.h"
#include "Console.h"
#include "Material.h"
#include "Math/Math.h"

#define THMDL_VERSION_3 0x0003
#define THMDL_VERSION_4 0x0004
#define THMDL_VERSION_5 0x0005
#define THMDL_VERSION_6 0x0006
#define THMDL_VERSION 0x0007

#define THMDL_MAGIC_SIZE 27
static const char* thmdlMagicStr = "\0\0ThoriumEngine Model File\0";

CModelAsset::~CModelAsset()
{
	ClearMeshes();
}

void CModelAsset::Init()
{
	TUniquePtr<IBaseFStream> stream = file->GetStream("rb");
	if (!stream || !stream->IsOpen())
	{
		CONSOLE_LogError("CModelAsset", FString("Failed to create file stream for '") + file->Path() + "'");
		return;
	}

	char magicStr[THMDL_MAGIC_SIZE];
	stream->Read(magicStr, THMDL_MAGIC_SIZE);

	if (memcmp(thmdlMagicStr, magicStr, THMDL_MAGIC_SIZE) != 0)
	{
		CONSOLE_LogError("CModelAsset", FString("Invalid Model file '") + file->Path() + "'");
		return;
	}

	*stream >> &fileVersion;

	//if (version != THMDL_VERSION)
	//{
	//	CONSOLE_LogError("CModelAsset", FString("Invalid Model file version '") + FString::ToString(version) + "'  expected version '" + FString::ToString(THMDL_VERSION) + "'");
	//	return;
	//}

	uint numMeshes;
	uint numMaterials;
	uint numBodyGroups;
	uint numColliders = 0;
	uint numConvexMeshes = 0;

	*stream >> &numMeshes >> &numMaterials >> &numLODs >> &numBodyGroups;

	if (fileVersion > THMDL_VERSION_6)
	{
		*stream >> &numColliders >> &numConvexMeshes;

		colliders.Resize(numColliders);
		convexMeshes.Resize(numConvexMeshes);
	}

	meshes.Resize(numMeshes);
	materials.Resize(numMaterials);
	bodyGroups.Resize(numBodyGroups);

	for (uint i = 0; i < numMeshes; i++)
	{
		meshes[i].meshDataOffset = stream->Tell();

		SizeType nextMesh;
		*stream >> &nextMesh;

		if (fileVersion > THMDL_VERSION_5)
			*stream >> meshes[i].meshName;

		SizeType numVertices;
		SizeType numIndices;

		*stream >> &numVertices >> &numIndices;

		if (fileVersion > THMDL_VERSION_5)
			*stream >> &meshes[i].topologyType;

		if (fileVersion > THMDL_VERSION_3)
			*stream >> &meshes[i].bounds;

		stream->Seek(nextMesh, SEEK_SET);
	}

	for (uint i = 0; i < numMaterials; i++)
	{
		*stream >> materials[i].name;
		*stream >> materials[i].path;
	}

	for (uint i = 0; i < numLODs; i++)
	{
		uint32 numIndices;
		*stream >> &numIndices;

		// fucking idiot forgot to save the distance bias.
		if (fileVersion > THMDL_VERSION_5)
			*stream >> &LODs[i].distanceBias;

		LODs[i].meshIndices.Resize(numIndices);

		for (uint32 x = 0; x < numIndices; x++)
			*stream >> &LODs[i].meshIndices[x];
	}

	for (uint i = 0; i < numBodyGroups; i++)
	{
		FBodyGroup& bg = bodyGroups[i];

		*stream >> bg.name;

		uint32 numOptions;
		*stream >> &numOptions;

		bg.options.Resize(numOptions);

		for (uint32 x = 0; x < numOptions; x++)
		{
			*stream >> bg.options[x].name;

			uint32 numIndices;
			*stream >> &numIndices;

			bg.options[x].meshIndices.Resize(numIndices);

			for (uint32 y = 0; y < numIndices; y++)
			{
				*stream >> &bg.options[x].meshIndices[y];
			}
		}
	}

	if (fileVersion > THMDL_VERSION_6)
	{
		for (uint i = 0; i < numColliders; i++)
		{
			FModelCollider& col = colliders[i];
			*stream >> &col;
		}

		for (uint i = 0; i < numConvexMeshes; i++)
		{
			TArray<FVector>& mesh = convexMeshes[i];
			uint vertices;
			*stream >> &vertices;
			mesh.Resize(vertices);

			for (uint v = 0; v < vertices; v++)
				*stream >> &mesh[v];
		}
	}

	uint32 numBones;
	*stream >> &numBones;

	skeleton.bones.Resize(numBones);

	for (uint32 i = 0; i < numBones; i++)
	{
		FBone& bone = skeleton.bones[i];

		*stream >> bone.name;
		*stream >> &bone.parent;
		*stream >> &bone.position >> &bone.rotation;
	}

	UpdateBoneMatrices();

	bInitialized = true;
}

void CModelAsset::Init(const TArray<FMesh>& m)
{
	numLODs = 0;
	meshes = m;
}

void CModelAsset::Save()
{
	// first check if we have any data to save.
	bool bLoadedMeshData = false;
	for (auto& mesh : meshes)
	{
		if (mesh.numIndexData % 3 != 0)
		{
			CONSOLE_LogError("CModelAsset", "Attempted to save ModelAsset with an invalid number of indices.");
			return;
		}

		if (!mesh.indexData || !mesh.vertexData)
		{
			if (gIsEditor)
			{
				LoadMeshData();
				bLoadedMeshData = true;
				break;
			}

			CONSOLE_LogError("CModelAsset", "Attempted to save ModelAsset but no mesh data was provided.");
			return;
		}
	}

	TUniquePtr<IBaseFStream> stream = file->GetStream("wb");
	if (!stream || !stream->IsOpen())
	{
		CONSOLE_LogError("CModelAsset", FString("Failed to create file stream for '") + file->Path() + "'");
		return;
	}

	stream->Write((void*)thmdlMagicStr, THMDL_MAGIC_SIZE);

	fileVersion = THMDL_VERSION;
	*stream << &fileVersion;

	uint numMeshes = (uint)meshes.Size();
	uint numMaterials = (uint)materials.Size();
	uint numBodyGroups = (uint)bodyGroups.Size();
	uint numColliders = (uint)colliders.Size();
	uint numConvexMeshes = (uint)convexMeshes.Size();

	*stream << &numMeshes << &numMaterials << &numLODs << &numBodyGroups << &numColliders << &numConvexMeshes;

	for (auto& mesh : meshes)
	{
		SizeType prevOffset = stream->Tell();
		*stream << &prevOffset; // just write anything for now.

		mesh.meshDataOffset = prevOffset;

		*stream << mesh.meshName;

		*stream << &mesh.numVertexData << & mesh.numIndexData;
		*stream << &mesh.topologyType;
		*stream << &mesh.bounds;

		for (SizeType i = 0; i < mesh.numVertexData; i++)
			*stream << &mesh.vertexData[i];

		for (SizeType i = 0; i < mesh.numIndexData; i++)
			*stream << &mesh.indexData[i];

		*stream << &mesh.materialIndex;

		SizeType curOffset = stream->Tell();
		stream->Seek(prevOffset, SEEK_SET);
		*stream << &curOffset;
		stream->Seek(curOffset, SEEK_SET);
	}

	for (uint i = 0; i < numMaterials; i++)
	{
		*stream << materials[i].name;
		*stream << materials[i].path;
	}

	for (uint i = 0; i < numLODs; i++)
	{
		uint32 numIndices = (uint32)LODs[i].meshIndices.Size();
		*stream << &numIndices;

		*stream << &LODs[i].distanceBias;

		for (uint32 x = 0; x < numIndices; x++)
			*stream << &LODs[i].meshIndices[x];
	}

	for (uint i = 0; i < numBodyGroups; i++)
	{
		FBodyGroup& bg = bodyGroups[i];

		*stream << bg.name;

		uint32 numOptions = (uint)bg.options.Size();
		*stream << &numOptions;

		for (uint32 x = 0; x < numOptions; x++)
		{
			*stream << bg.options[x].name;

			uint32 numIndices = (uint)bg.options[x].meshIndices.Size();
			*stream << &numIndices;

			for (uint32 y = 0; y < numIndices; y++)
			{
				*stream << &bg.options[x].meshIndices[y];
			}
		}
	}

	for (uint i = 0; i < numColliders; i++)
	{
		FModelCollider& col = colliders[i];
		*stream << &col;
	}

	for (uint i = 0; i < numConvexMeshes; i++)
	{
		TArray<FVector>& mesh = convexMeshes[i];
		uint numV = (uint)mesh.Size();
		*stream << &numV;

		for (uint v = 0; v < numV; v++)
			*stream << &mesh[v];
	}

	uint32 numBones = (uint32)skeleton.bones.Size();
	*stream << &numBones;

	for (uint32 i = 0; i < numBones; i++)
	{
		FBone& bone = skeleton.bones[i];

		*stream << bone.name;
		*stream << &bone.parent;
		*stream << &bone.position << &bone.rotation;
	}

	if (bLoadedMeshData)
		ClearMeshData();
}

void CModelAsset::Load(uint8 lodLevel)
{
	if (!bRegistered)
		return;

	if (!bInitialized)
	{
		//CONSOLE_LogError("Attempted to load ModelAsset while asset was not initialized properly");
		return;
	}

	if (IsLodLoaded(lodLevel))
		return;

	TUniquePtr<IBaseFStream> stream = file->GetStream("rb");
	if (!stream || !stream->IsOpen())
	{
		CONSOLE_LogError("CModelAsset", FString("Failed to create file stream for '") + file->Path() + "'");
		return;
	}

	TArray<uint32> meshesToLoad;

	if (numLODs == 0)
	{
		meshesToLoad.Resize(meshes.Size());
		for (SizeType i = 0; i < meshesToLoad.Size(); i++)
			meshesToLoad[i] = (uint32)i;
	}
	else
	{
		uint8 lod = FMath::Clamp(lodLevel, (uint8)0, numLODs);

		meshesToLoad.Reserve(LODs[lod].meshIndices.Size());

		for (auto& index : LODs[lod].meshIndices)
			meshesToLoad.Add(index);
	}

	for (auto it : meshesToLoad)
	{
		stream->Seek(meshes[it].meshDataOffset, SEEK_SET);

		SizeType nextOffset;

		SizeType numVertices;
		SizeType numIndices;
		TArray<FVertex> vertices;
		TArray<uint> indices;

		*stream >> &nextOffset;

		FString tempName;
		
		if (fileVersion > THMDL_VERSION_5)
			*stream >> tempName;

		*stream >> &numVertices >> &numIndices;

		if (fileVersion > THMDL_VERSION_5)
			*stream >> &meshes[it].topologyType;

		if (fileVersion > THMDL_VERSION_3)
			*stream >> &meshes[it].bounds;

		vertices.Resize(numVertices);
		indices.Resize(numIndices);

		for (SizeType i = 0; i < numVertices; i++)
			*stream >> &vertices[i];

		for (SizeType i = 0; i < numIndices; i++)
			*stream >> &indices[i];

		meshes[it].vertexBuffer = Renderer::CreateVertexBuffer(vertices);
		meshes[it].indexBuffer = Renderer::CreateIndexBuffer(indices);
		meshes[it].numVertices = (uint)numVertices;
		meshes[it].numIndices = (uint)numIndices;

		// keep vertex data loaded in ram if we're running the editor
		if (gIsEditor)
		{
			meshes[it].vertexData = new FVertex[vertices.Size()];
			meshes[it].numVertexData = vertices.Size();
			memcpy(meshes[it].vertexData, vertices.Data(), vertices.Size() * sizeof(FVertex));

			meshes[it].indexData = new uint[indices.Size()];
			meshes[it].numIndexData = indices.Size();
			memcpy(meshes[it].indexData, indices.Data(), indices.Size() * sizeof(uint));
		}

		*stream >> &meshes[it].materialIndex;

		FMaterial& mat = materials[meshes[it].materialIndex];
		mat.obj = CResourceManager::GetResource<CMaterial>(mat.path);
	}

	SetLodLevel(lodLevel, true);
}

void CModelAsset::Unload(uint8 lodLevel)
{
	if (!IsLodLoaded(lodLevel) || numLODs == 0)
		return;

	for (auto it : LODs[FMath::Clamp(lodLevel, (uint8)0, numLODs)].meshIndices)
	{
		meshes[it].vertexBuffer->Delete();
		meshes[it].indexBuffer->Delete();
	}

	SetLodLevel(lodLevel, false);
}

void CModelAsset::LoadMeshData()
{
	if (!bInitialized || !file)
		return;

	TUniquePtr<IBaseFStream> stream = file->GetStream("rb");
	if (!stream || !stream->IsOpen())
	{
		CONSOLE_LogError("CModelAsset", FString("Failed to create file stream for '") + file->Path() + "'");
		return;
	}

	for (auto& mesh : meshes)
	{
		stream->Seek(mesh.meshDataOffset, SEEK_SET);

		SizeType nextOffset;

		SizeType numVertices;
		SizeType numIndices;

		*stream >> &nextOffset;
		
		FString tempName;

		if (fileVersion > THMDL_VERSION_5)
			*stream >> tempName;
		
		*stream >> &numVertices >> &numIndices;

		if (fileVersion > THMDL_VERSION_5)
			*stream >> &mesh.topologyType;

		if (fileVersion > THMDL_VERSION_3)
			*stream >> &mesh.bounds;

		FVertex* vertices = new FVertex[numVertices];
		uint* indices = new uint[numIndices];

		for (SizeType i = 0; i < numVertices; i++)
			*stream >> &vertices[i];

		for (SizeType i = 0; i < numIndices; i++)
			*stream >> &indices[i];

		mesh.numVertexData = numVertices;
		mesh.numIndexData = numIndices;
		mesh.vertexData = vertices;
		mesh.indexData = indices;
	}
}

void CModelAsset::ClearMeshData()
{
	for (auto& mesh : meshes)
	{
		if (mesh.vertexData)
			delete[] mesh.vertexData;
		if (mesh.indexData)
			delete[] mesh.indexData;

		mesh.vertexData = nullptr;
		mesh.indexData = nullptr;
	}
}

FBounds CModelAsset::CalculateBounds()
{
	bounds = FBounds();
	for (auto& mesh : meshes)
		bounds = bounds.Combine(mesh.bounds);
	return bounds;
}

FBone* CModelAsset::GetBone(const FString& name)
{
	for (auto& b : skeleton.bones)
	{
		if (b.name == name)
			return &b;
	}
	return nullptr;
}

SizeType CModelAsset::GetBoneIndex(const FString& name)
{
	for (SizeType i = 0; i < skeleton.bones.Size(); i++)
	{
		if (skeleton.bones[i].name == name)
			return i;
	}
	return -1;
}

int CModelAsset::GetLodFromDistance(float distance)
{
	for (int8 i = numLODs; i > 0; i--)
	{
		if (distance > LODs[i - 1].distanceBias)
		{
			//if (IsLodLoaded(i - 1))
				return i - 1;
		}
	}
	return FMath::Max(0, (int)numLODs - 1);
}

void CModelAsset::ClearMeshes()
{
	for (auto& mesh : meshes)
	{
		if (mesh.vertexBuffer)
			mesh.vertexBuffer->Delete();
		if (mesh.indexBuffer)
			mesh.indexBuffer->Delete();

		if (mesh.vertexData)
			delete[] mesh.vertexData;
		if (mesh.indexData)
			delete[] mesh.indexData;
	}
	meshes.Clear();
}

void CModelAsset::UpdateBoneMatrices()
{
	skeleton.invModel.Resize(skeleton.bones.Size());

	for (SizeType i = 0; i < skeleton.bones.Size(); i++)
	{
		FMatrix local = (FMatrix(1.f).Translate(skeleton.bones[i].position) * skeleton.bones[i].rotation);
		skeleton.invModel[i] = skeleton.bones[i].parent != -1 ? skeleton.invModel[skeleton.bones[i].parent] * local : local;
	}

	for (SizeType i = 0; i < skeleton.bones.Size(); i++)
	{
		skeleton.invModel[i] = skeleton.invModel[i].Inverse();
	}
}

void FMesh::CalculateBounds()
{
	if (!vertexData)
		return;

	FVector min;
	FVector max;

	for (uint32 i = 0; i < numVertices; i++)
	{
		FVector& vert = vertexData[i].position;

		if (vert.x < min.x)
			min.x = vert.x;
		if (vert.y < min.y)
			min.y = vert.y;
		if (vert.z < min.z)
			min.z = vert.z;

		if (vert.x > max.x)
			max.x = vert.x;
		if (vert.y > max.y)
			max.y = vert.y;
		if (vert.z > max.z)
			max.z = vert.z;
	}

	bounds = FBounds::FromMinMax(min, max);
}
