
#include "Renderer.h"
#include "GraphicsInterface.h"
#include "RenderScene.h"
#include "Assets/Material.h"
#include "Game/Components/CameraComponent.h"
#include "Console.h"
#include "DebugRenderer.h"
#include "PostProcessing.h"
#include <map>

#include <algorithm>
#include <thread>

IRenderer* gRenderer = nullptr;
IGraphicsInterface* gGHI = nullptr;

FRenderStatistics gRenderStats;

std::mutex IRenderer::gpuMutex;

std::thread renderThread;

// 0 = None, 1 = Unlit, 2 = Normal, 3 = Material
CConVar cvRenderMaterialMode("r.materialmode", 0, CConVar::SERVER_CHEAT);

CConVar cvRenderShadowEnabled("r.shadow.enabled", "config/graphics.cfg", 1, 0, 1);
CConVar cvRenderShadowQuality("r.shadow.quality", "config/graphics.cfg", 4, 0, 4);
CConVar cvRenderTextureQuality("r.texture.quality", "config/graphics.cfg", 4, 0, 4);

CConVar cvRenderScreenPercentage("r.screen_percentage", "config/graphics.cfg", 100.f, 12.5f, 400.f);

// Screen space Ambient Occlusion.
CConVar cvRenderSsaoEnabled("r.ssao.enabled", "config/graphics.cfg", 1);
CConVar cvRenderSsaoQuality("r.ssao.quality", "config/graphics.cfg", 4, 0, 4);

// Screen space shadows.
CConVar cvRenderSsShadows("r.ssshadows.enabled", "config/graphics.cfg", 1);
CConVar cvRenderSsShadowsQuality("r.ssshadows.quality", "config/graphics.cfg", 4, 0, 4);

// Bloom
CConVar cvRenderBloomEnabled("r.bloom.enabled", "config/graphics.cfg", 1);
CConVar cvRenderBloomIntensity("r.bloom.intensity", "config/graphics.cfg", 0.25f);
CConVar cvRenderBloomThreshold("r.bloom.threshold", "config/graphics.cfg", 1.f);
CConVar cvRenderBloomKnee("r.bloom.knee", "config/graphics.cfg", 0.75f);

CConVar cvRenderFBPointFilter("r.framebuffer.pointfilter", "config/graphics.cfg", 0, 0, 1);

CConVar cvForceForwardRendering("r.forceforward", "config/graphics.cfg", 0, 0, 1);

IRenderer::IRenderer()
{
	gRenderer = this;
}

static void DoRenderMT()
{
	gRenderer->Render();
}

void IRenderer::RenderMT()
{
	// TODO: Render in a seperate thread.
	renderThread = std::thread(&DoRenderMT);
}

void IRenderer::JoinRenderThread()
{
	renderThread.join();
}

TIterator<FRenderCommand> IRenderer::__getRenderCommands(ERenderPass currentPass, TIterator<FRenderCommand> begin, TIterator<FRenderCommand> end, TArray<FRenderCommand>& out)
{
	// We expect that the render commands are given in order of each render pass -
	// so we don't have to iterate over all objects in the array.
	for (auto it = begin; it != end; it++)
	{
		if (it->renderPass == currentPass)
			out.Add(*it);
		else
			return it;
	}
	return end;
}

static TObjectPtr<IShaderBuffer> blitDataBuffer = nullptr;

void IRenderer::Blit(IFrameBuffer* a, IFrameBuffer* b)
{
	int w, h;
	a->GetSize(w, h);

	int widthB, heightB;
	b->GetSize(widthB, heightB);

	float sizeScalar[4] = { 0, 0, (float)widthB / float(w), (float)heightB / float(h) };

	if (!blitDataBuffer)
		blitDataBuffer = gGHI->CreateShaderBuffer(nullptr, sizeof(float) * 4);
	
	blitDataBuffer->Update(sizeof(float) * 4, sizeScalar);

	gGHI->SetViewport(0, 0, (float)widthB, (float)heightB);
	gGHI->SetFrameBuffer(b);
	gGHI->SetShaderResource(a, 1);
	gGHI->SetShaderBuffer(blitDataBuffer, 0);
	gGHI->SetVsShader(gRenderer->shaderScreenPlane->GetShader(ShaderType_Vertex));
	gGHI->SetPsShader(gRenderer->shaderBlit->GetShader(ShaderType_Fragment));

	FMesh mesh;
	mesh.numVertices = 3;
	gGHI->DrawMesh(&mesh);
}

void IRenderer::Blit(IFrameBuffer* a, IFrameBuffer* b, FVector2 viewportPos, FVector2 viewportScale)
{
	int w, h;
	a->GetSize(w, h);

	int widthB, heightB;
	b->GetSize(widthB, heightB);

	float sizeScalar[4] = { viewportPos.x, viewportPos.y, viewportScale.x, viewportScale.y };

	if (!blitDataBuffer)
		blitDataBuffer = gGHI->CreateShaderBuffer(nullptr, sizeof(float) * 4);

	blitDataBuffer->Update(sizeof(float) * 4, sizeScalar);

	gGHI->SetViewport(0, 0, (float)widthB, (float)heightB);
	gGHI->SetFrameBuffer(b);
	gGHI->SetShaderResource(a, 1);
	gGHI->SetShaderBuffer(blitDataBuffer, 0);
	gGHI->SetVsShader(gRenderer->shaderScreenPlane->GetShader(ShaderType_Vertex));
	gGHI->SetPsShader(gRenderer->shaderBlit->GetShader(ShaderType_Fragment));

	FMesh mesh;
	mesh.numVertices = 3;
	gGHI->DrawMesh(&mesh);
}

void IRenderer::Blit(IFrameBuffer* a, IFrameBuffer* b, int destinationMip)
{
	int w, h;
	a->GetSize(w, h);

	int widthB, heightB;
	b->GetSize(widthB, heightB);

	float sizeScalar[4] = { 0, 0, (float)widthB / float(w), (float)heightB / float(h) };

	if (!blitDataBuffer)
		blitDataBuffer = gGHI->CreateShaderBuffer(nullptr, sizeof(float) * 4);

	blitDataBuffer->Update(sizeof(float) * 4, sizeScalar);

	gGHI->SetViewport(0, 0, (float)widthB, (float)heightB);
	gGHI->SetFrameBuffer(b, destinationMip);
	gGHI->SetShaderResource(a, 1);
	gGHI->SetShaderBuffer(blitDataBuffer, 0);
	gGHI->SetVsShader(gRenderer->shaderScreenPlane->GetShader(ShaderType_Vertex));
	gGHI->SetPsShader(gRenderer->shaderBlit->GetShader(ShaderType_Fragment));

	FMesh mesh;
	mesh.numVertices = 3;
	gGHI->DrawMesh(&mesh);
}

void IRenderer::Blit(IFrameBuffer* a, IFrameBuffer* b, int destinationMip, FVector2 viewportPos, FVector2 viewportScale)
{
	int w, h;
	a->GetSize(w, h);

	int widthB, heightB;
	b->GetSize(widthB, heightB);

	float sizeScalar[4] = { viewportPos.x, viewportPos.y, viewportScale.x, viewportScale.y };

	if (!blitDataBuffer)
		blitDataBuffer = gGHI->CreateShaderBuffer(nullptr, sizeof(float) * 4);

	blitDataBuffer->Update(sizeof(float) * 4, sizeScalar);

	gGHI->SetViewport(0, 0, (float)widthB, (float)heightB);
	gGHI->SetFrameBuffer(b, destinationMip);
	gGHI->SetShaderResource(a, 1);
	gGHI->SetShaderBuffer(blitDataBuffer, 0);
	gGHI->SetVsShader(gRenderer->shaderScreenPlane->GetShader(ShaderType_Vertex));
	gGHI->SetPsShader(gRenderer->shaderBlit->GetShader(ShaderType_Fragment));

	FMesh mesh;
	mesh.numVertices = 3;
	gGHI->DrawMesh(&mesh);
}

void IRenderer::Init()
{
	debugUnlit = CShaderSource::GetShaderSource("Unlit");
	if (debugUnlit)
		debugUnlit->LoadShaderObjects();

	debugNormalForward = CShaderSource::GetShaderSource("DebugNormalForward");
	if (debugNormalForward)
		debugNormalForward->LoadShaderObjects();
}

void IRenderer::renderAll()
{
	// do this so we don't crash, so we can still use the engine.
	if (!bInitialized)
		return;

	if (gDebugRenderer)
		gDebugRenderer->Render();

	TArray<CRenderScene*>& renderQueue = gRenderer->renderScenes;

	// reset render stats
	gRenderStats = FRenderStatistics();

	// Draw each scene.
	for (auto scene : renderQueue)
	{
		gRenderer->curScene = scene;
		RenderShadowMaps(scene);

		for (auto* cam : scene->cameras)
			if (cam != scene->GetPrimaryCamera() && cam->bEnabled)
				RenderCamera(scene, cam);

		if (scene->GetPrimaryCamera())
			RenderCamera(scene, scene->GetPrimaryCamera());
		// else draw error msg.

		RenderUserInterface(scene);

		gRenderStats.totalPrimitives += scene->GetPrimitives().Size();

		scene->renderQueue.Clear();
	}
	gRenderer->curCamera = nullptr;
	gRenderer->curScene = nullptr;

	gRenderer->renderScenes.Clear();
}

#if PLATFORM_WINDOWS
#include "Platform/Windows/DirectX/DirectXInterface.h"
#endif

IGraphicsInterface* GetGraphicsInterface(EGraphicsApi api)
{
	if (api == EGraphicsApi::DEFAULT)
#if PLATFORM_WINDOWS
		api = EGraphicsApi::DIRECTX_11;
#else
		api = EGraphicsApi::VULKAN;
#endif

#if PLATFORM_WINDOWS
	if (api == EGraphicsApi::DIRECTX_11)
		return new DirectXInterface();
#endif

	return nullptr;
}
