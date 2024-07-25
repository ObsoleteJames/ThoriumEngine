#pragma once

#include "Math/Vectors.h"
#include "Math/Bounds.h"

class IVertexBuffer;
class IIndexBuffer;

struct FVertex
{
	FVector position;
	FVector normal;
	FVector tangent;
	FVector color;
	float uv1[2];
	float uv2[2];

	int bones[4];
	float boneInfluence[4];
};

struct ENGINE_API FMesh
{
	enum ETopologyType
	{
		TOPOLOGY_TRIANGLES,
		TOPOLOGY_LINES,
		TOPOLOGY_POINTS,
	};

public:
	TObjectPtr<IVertexBuffer> vertexBuffer;
	TObjectPtr<IIndexBuffer> indexBuffer;

	uint32 numIndices;
	uint32 numVertices;
	uint32 materialIndex;

	uint8 topologyType = TOPOLOGY_TRIANGLES;
	FString meshName;

	FBounds bounds;

public:
	void CalculateBounds();

public:
	// This is only used for saving new mesh data.
	SizeType numVertexData;
	FVertex* vertexData = nullptr;

	SizeType numIndexData;
	uint* indexData = nullptr;

	SizeType meshDataOffset;
};
