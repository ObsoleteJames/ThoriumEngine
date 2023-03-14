
#include "RenderCommands.h"

FRenderCommand::FRenderCommand(const FDrawMeshCmd& cmd, ERenderPass rp) : type(DRAW_MESH), renderPass(rp)
{
	new (&drawMesh) FDrawMeshCmd(cmd);
}

FRenderCommand::FRenderCommand(const FDrawTextCmd& cmd, ERenderPass rp) : type(DRAW_TEXT), renderPass(rp)
{
	new (&drawText) FDrawTextCmd(cmd);
}

FRenderCommand::FRenderCommand(const FPostProcessEffectCmd& cmd) : type(POSTPROCESS_EFFECT), renderPass(R_POSTPROCESS_PASS)
{
	new (&postProcessEffect) FPostProcessEffectCmd(cmd);
}

FRenderCommand::FRenderCommand(const FRenderCommand& other) : type(other.type), renderPass(other.renderPass)
{
	switch (type)
	{
	case DRAW_MESH:
		new (&drawMesh) FDrawMeshCmd(other.drawMesh);
		break;
	case DRAW_TEXT:
		new (&drawText) FDrawTextCmd(other.drawText);
		break;
	case POSTPROCESS_EFFECT:
		new (&postProcessEffect) FPostProcessEffectCmd(other.postProcessEffect);
		break;
	}
}

FRenderCommand::~FRenderCommand()
{
	switch (type)
	{
	case DRAW_MESH:
		drawMesh.~FDrawMeshCmd();
		break;
	case DRAW_TEXT:
		drawText.~FDrawTextCmd();
		break;
	case POSTPROCESS_EFFECT:
		postProcessEffect.~FPostProcessEffectCmd();
		break;
	}
}
