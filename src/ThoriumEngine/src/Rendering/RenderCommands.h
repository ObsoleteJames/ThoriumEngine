#pragma once

#include "Math/Vectors.h"

struct FMesh;
struct FFont;
class CMaterial;
class IShader;

enum ERenderPass : uint8
{
	RENDER_PASS_NONE = 0,
	R_SHADOW_PASS = 1,
	R_DEFERRED_PASS = 1 << 1,
	R_FORWARD_PASS = 1 << 2,
	R_TRANSPARENT_PASS = 1 << 3,
	R_POSTPROCESS_PASS = 1 << 4,
	R_DEBUG_PASS = 1 << 5,
	R_UI_PASS = 1 << 6
};

enum EMeshDrawType : uint8
{
	MESH_DRAW_NONE = 0,
	MESH_DRAW_WIREFRAME = 1,
	MESH_DRAW_PRIMITIVE_LINES = 1 << 1,
	MESH_DRAW_PRIMITIVE_POINTS = 1 << 2,

	MESH_DRAW_DEFAULT = MESH_DRAW_NONE,
};

struct ENGINE_API FDrawMeshCmd
{
	FMatrix transform;
	TArray<FMatrix> skeletonMatrices;

	FMesh* mesh;
	CMaterial* material;

	int16 drawOrder = 0; // Draw order priority scalar.
	uint8 drawType;
};

struct ENGINE_API FDrawTextCmd
{
	FMatrix transform;

	FString text;
	FFont* font;

	int16 drawOrder = 0;
};

struct ENGINE_API FPostProcessEffectCmd
{
	IShader* pixelShader;
};

struct ENGINE_API FRenderCommand
{
	enum EType : uint8
	{
		INVALID,
		DRAW_MESH,
		DRAW_TEXT,
		POSTPROCESS_EFFECT
	};

public:
	FRenderCommand() : type(INVALID) { }
	FRenderCommand(const FDrawMeshCmd& cmd, ERenderPass rp);
	FRenderCommand(const FDrawTextCmd& cmd, ERenderPass rp);
	FRenderCommand(const FPostProcessEffectCmd& cmd);
	FRenderCommand(const FRenderCommand& other);
	~FRenderCommand();

	union
	{
		FDrawMeshCmd drawMesh;
		FDrawTextCmd drawText;
		FPostProcessEffectCmd postProcessEffect;
	};

	EType type;
	ERenderPass renderPass;
};
