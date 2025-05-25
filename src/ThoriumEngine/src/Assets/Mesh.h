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

	uint32 numIndices = 0;
	uint32 numVertices = 0;
	uint32 materialIndex = 0;

	uint8 topologyType = TOPOLOGY_TRIANGLES;
	FString meshName;

	FBounds bounds;

public:
	void CalculateBounds();

public:
	// This is only used for saving new mesh data.
	SizeType numVertexData = 0;
	FVertex* vertexData = nullptr;

	SizeType numIndexData = 0;
	uint* indexData = nullptr;

	SizeType meshDataOffset = 0;
};
