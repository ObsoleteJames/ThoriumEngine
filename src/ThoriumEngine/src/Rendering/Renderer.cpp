
#include "Renderer.h"
#include "RenderScene.h"
#include "Resources/Material.h"
#include "Game/Components/CameraComponent.h"
#include "Console.h"
#include "DebugRenderer.h"
#include "PostProcessing.h"
#include <map>

#include <algorithm>

IRenderer* gRenderer = nullptr;
std::mutex IRenderer::gpuMutex;

std::thread renderThread;

// 0 = None, 1 = Unlit, 2 = Normal, 3 = Material
static CConVar cvRenderMaterialMode("r.materialmode", 0, CConVar::SERVER_CHEAT);

static CConVar cvRenderShadowEnabled("r.shadow.enabled", "config/graphics.cfg", 1, 0, 1);
static CConVar cvRenderShadowQuality("r.shadow.quality", "config/graphics.cfg", 4, 0, 4);
static CConVar cvRenderTextureQuality("r.texture.quality", "config/graphics.cfg", 4, 0, 4);

static CConVar cvRenderScreenPercentage("r.screen_percentage", "config/graphics.cfg", 100.f, 50.f, 200.f);

// Screen space Ambient Occlusion.
static CConVar cvRenderSsaoEnabled("r.ssao.enabled", "config/graphics.cfg", 1);
static CConVar cvRenderSsaoQuality("r.ssao.quality", "config/graphics.cfg", 4, 0, 4);

// Screen space shadows.
static CConVar cvRenderSsShadows("r.ssshadows.enabled", "config/graphics.cfg", 1);
static CConVar cvRenderSsShadowsQuality("r.ssshadows.quality", "config/graphics.cfg", 4, 0, 4);

static FPostProcessSettings defaultPostProcess;

IRenderer::IRenderer()
{
	gRenderer = this;
}

void IRenderer::BeginRender()
{
	//renderScenes.Clear();
}

void IRenderer::RenderMT()
{
	// TODO: Render in a seperate thread.
	renderThread = std::thread(&IRenderer::renderAll);
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

void GetRenderCommands(ERenderPass renderPass, const TArray<FRenderCommand>& cmds, TArray<FRenderCommand>& out)
{
	for (auto& cmd : cmds)
	{
		if (cmd.renderPass == renderPass)
			out.Add(cmd);
	}
}

void GetMeshesToDraw(ERenderPass rp, const TArray<TPair<CPrimitiveProxy*, FMeshBuilder>>& meshes, TArray<TPair<CPrimitiveProxy*, FMeshBuilder::FRenderMesh*>>& out)
{
	for (auto& m : meshes)
	{
		for (auto& mesh : m.Value.GetMeshes())
		{
			if (mesh.rp != rp)
				continue;

			out.Add({ m.Key, &mesh });
		}
	}
}

void CreateForwardLightBuffer(const FVector& objPos, const TArray<CLightProxy*>& lights, FForwardLightsBuffer& out)
{
	std::multimap<float, CLightProxy*> sortedLights;

	for (auto* l : lights)
	{
		if (!l->Enabled())
			continue;

		if (l->type == CLightProxy::DIRECTIONAL_LIGHT)
		{
			if (out.numDirLights > 1)
				continue;

			out.dirLights[out.numDirLights].direction = l->direction;
			out.dirLights[out.numDirLights].color = l->color;
			out.dirLights[out.numDirLights].intensity = l->intensity;
			out.dirLights[out.numDirLights].shadowIndex = l->shadowIndex;
			out.numDirLights++;
			continue;
		}

		float distance = FMath::Abs(FVector::Distance(l->position, objPos));
		sortedLights.insert(std::pair(distance, l));
	}

	for (auto& l : sortedLights)
	{
		CLightProxy* light = l.second;
		if (light->type == CLightProxy::POINT_LIGHT)
		{
			if (out.numPointLights == 8)
				continue;

			out.pointLights[out.numPointLights].position = light->position;
			out.pointLights[out.numPointLights].color = light->color;
			out.pointLights[out.numPointLights].intensity = light->intensity;
			out.pointLights[out.numPointLights].range = light->range;
			out.pointLights[out.numPointLights].shadowIndex = light->shadowIndex;
			out.numPointLights++;
		}
		else
		{
			if (out.numSpotLights == 8)
				continue;

			out.spotLights[out.numSpotLights].position = light->position;
			out.spotLights[out.numSpotLights].direction = light->direction;
			out.spotLights[out.numSpotLights].color = light->color;
			out.spotLights[out.numSpotLights].intensity = light->intensity;
			out.spotLights[out.numSpotLights].innerConeAngle = light->innerConeAngle;
			out.spotLights[out.numSpotLights].outerConeAngle = light->outerConeAngle;
			out.spotLights[out.numSpotLights].range = light->range;
			out.spotLights[out.numSpotLights].shadowIndex = light->shadowIndex;
			out.numSpotLights++;
		}
	}
}

void IRenderer::Init()
{
	//CResourceManager::LoadResources<CShaderSource>();

	debugUnlit = CShaderSource::GetShader("Unlit");
	debugNormalForward = CShaderSource::GetShader("DebugNormalForward");

	shaderScreenPlane = CShaderSource::GetShader("ScreenPlaneVS");
	shaderBlit = CShaderSource::GetShader("blitFrameBuffer");

	debugUnlit->LoadShaderObjects();
	debugNormalForward->LoadShaderObjects();

	shaderScreenPlane->LoadShaderObjects();
	shaderBlit->LoadShaderObjects();

	sceneBuffer = gRenderer->CreateShaderBuffer(nullptr, sizeof(FSceneInfoBuffer));
	objectBuffer = gRenderer->CreateShaderBuffer(nullptr, sizeof(FObjectInfoBuffer));
	forwardLightsBuffer = gRenderer->CreateShaderBuffer(nullptr, sizeof(FForwardLightsBuffer));
	forwardShadowDataBuffer = gRenderer->CreateShaderBuffer(nullptr, sizeof(FShadowDataBuffer));

	int shadowQuality = cvRenderShadowQuality.AsInt() + 1;
	shadowTexSize = 512 * (shadowQuality < 5 ? shadowQuality : 8);

	cvRenderShadowQuality.onValueChanged.Bind([=](float v) {
		int shadowQuality = int(v) + 1;
		shadowTexSize = 512 * (shadowQuality < 5 ? shadowQuality : 8);

		FDepthBufferInfo sunDepth{};
		sunDepth.width = shadowTexSize * 4;
		sunDepth.height = shadowTexSize;
		sunDepth.bShaderResource = true;
		sunDepth.format = TH_DBF_R16;
		sunDepth.arraySize = 1;

		delete sunLightShadows;
		sunLightShadows = gRenderer->CreateDepthBuffer(sunDepth);
	});

	FDepthBufferInfo sunDepth{};
	sunDepth.width = shadowTexSize * 4;
	sunDepth.height = shadowTexSize;
	sunDepth.bShaderResource = true;
	sunDepth.format = TH_DBF_R16;
	sunDepth.arraySize = 1;

	sunLightShadows = gRenderer->CreateDepthBuffer(sunDepth);

	gDebugRenderer = new CDebugRenderer();
}

void IRenderer::Blit(IFrameBuffer* a, IFrameBuffer* b)
{
	int w, h;
	a->GetSize(w, h);

	gRenderer->SetViewport(0, 0, (float)w, (float)h);
	gRenderer->SetFrameBuffer(b);
	gRenderer->SetShaderResource(a, 1);
	gRenderer->SetVsShader(gRenderer->shaderScreenPlane->vsShader);
	gRenderer->SetPsShader(gRenderer->shaderBlit->psShader);

	FMesh mesh;
	mesh.numVertices = 3;
	gRenderer->DrawMesh(&mesh);
}

void IRenderer::renderAll()
{
	if (gDebugRenderer)
		gDebugRenderer->Render();

	TArray<CRenderScene*>& renderQueue = gRenderer->renderScenes;

	// Draw each scene.
	for (auto scene : renderQueue)
	{
		RenderShadowMaps(scene);

		for (auto* cam : scene->cameras)
			if (cam != scene->GetPrimaryCamera() && cam->bEnabled)
				RenderCamera(scene, cam);

		if (scene->GetPrimaryCamera())
			RenderCamera(scene, scene->GetPrimaryCamera());
		// else draw error msg.

		RenderUserInterface(scene);

		scene->renderQueue.Clear();
	}

	gRenderer->renderScenes.Clear();
}

void IRenderer::RenderCamera(CRenderScene* scene, CCameraProxy* camera)
{
	static TArray<FRenderCommand> curCommands;
	SizeType queueLastIndex = 0;

	int viewWidth, viewHeight;
	scene->frameBuffer->GetSize(viewWidth, viewHeight);
	scene->frameBuffer->Clear();
	
	scene->colorBuffer->Clear();

	if (viewWidth > scene->GetFrameBufferWidth() || viewHeight > scene->GetFrameBufferHeight())
		scene->ResizeBuffers(viewWidth, viewHeight);

	gRenderer->SetViewport(0.f, 0.f, (float)viewWidth, (float)viewHeight);

	camera->CalculateMatrix((float)viewWidth / (float)viewHeight);

	FSceneInfoBuffer sceneInfo{ camera->projection * camera->view,
		camera->view, camera->projection,
		camera->position, 0u, camera->GetForwardVector(), 0u, scene->GetTime(), FVector2(viewWidth, viewHeight) / FVector2(scene->bufferWidth, scene->bufferHeight) };
	gRenderer->sceneBuffer->Update(sizeof(FSceneInfoBuffer), &sceneInfo);

	static TArray<TPair<CPrimitiveProxy*, FMeshBuilder>> dynamicMeshes;
	dynamicMeshes.Clear();

	for (auto* primitive : scene->primitves)
	{
		if (!primitive->IsVisible())
			continue;

		dynamicMeshes.Add({ primitive, FMeshBuilder() });
		FMeshBuilder& dynamic = dynamicMeshes.last()->Value;

		primitive->GetDynamicMeshes(dynamic);
	}

	// ------------- SHADOW PASS -------------
	//{
	//	{
	//		auto end = __getRenderCommands(R_SHADOW_PASS, scene->renderQueue.At(queueLastIndex), scene->renderQueue.end(), curCommands);
	//		queueLastIndex = ((SizeType)end.ptr - (SizeType)scene->renderQueue.Data()) / sizeof(FRenderCommand);
	//	}
	//
	//	ILightComponent* sunLight;
	//
	//	// Calculate shadow-map priority
	//	std::vector<TPair<float, ILightComponent*>> shadowLights;
	//	for (auto* light : scene->lights)
	//	{
	//		if (!light->CastShadows())
	//			continue;
	//
	//		if (light->IsSunlight())
	//		{
	//			sunLight = light;
	//			continue;
	//		}
	//
	//		float distance = FVector::Distance(scene->GetCamera()->GetWorldPosition(), light->GetWorldPosition());
	//		shadowLights.push_back({ distance, light });
	//	}
	//
	//	struct {
	//		bool operator()(const TPair<float, ILightComponent*>& a, const TPair<float, ILightComponent*>& b) {
	//			return a.Key < b.Key;
	//		}
	//	} FCustomSort;
	//
	//	std::sort(shadowLights.begin(), shadowLights.end(), FCustomSort);
	//
	//	constexpr int maxShadows = 16;
	//	for (int i = 0; i < FMath::Min(16, (int)shadowLights.size()); i++)
	//	{
	//		// TODO: render shadows.
	//	}
	//}

#if 0
	// ------------- DEFERRED PASS -------------
	curCommands.Clear();
	GetRenderCommands(R_DEFERRED_PASS, scene->renderQueue, curCommands);

	LockGPU();
	gRenderer->BindGBuffer();

	gRenderer->SetPsShader(gRenderer->psShaderDeferred);
	gRenderer->BindGlobalData();

	for (auto& rc : curCommands)
	{
		if (rc.type != FRenderCommand::DRAW_MESH)
			continue;

		gRenderer->SetVsShader(rc.drawMesh.material->GetVsShader());
		gRenderer->DrawMesh(&rc.drawMesh);
}

	gRenderer->SetFrameBuffer(scene->frameBuffer, nullptr);
	gRenderer->SetPsShader(gRenderer->psShaderDeferredLighting);
	gRenderer->SetVsShader(gRenderer->vsShaderDeferredLighting);

	// TODO: Setup global buffer information.

	gRenderer->DrawMesh(gRenderer->quadMesh);

	gRenderer->SetVsShader(nullptr);
#endif

	Blit(scene->colorBuffer, scene->preTranslucentBuff);
	gRenderer->SetViewport(0.f, 0.f, (float)viewWidth, (float)viewHeight);

	// ------------- FORWARD PASS -------------
	{
		curCommands.Clear();
		GetRenderCommands(R_FORWARD_PASS, scene->renderQueue, curCommands);

		static TArray<TPair<CPrimitiveProxy*, FMeshBuilder::FRenderMesh*>> forwardMeshes;
		forwardMeshes.Clear();
		GetMeshesToDraw(R_FORWARD_PASS, dynamicMeshes, forwardMeshes);

		LockGPU();
		gRenderer->SetBlendMode(EBlendMode::BLEND_DISABLED);

		gRenderer->SetFrameBuffer(scene->colorBuffer, scene->depth);
		gRenderer->SetShaderBuffer(gRenderer->sceneBuffer, 1);
		gRenderer->SetShaderBuffer(gRenderer->forwardLightsBuffer, 2);
		gRenderer->SetShaderBuffer(gRenderer->objectBuffer, 3);
		gRenderer->SetShaderBuffer(gRenderer->forwardShadowDataBuffer, 4);
		
		gRenderer->SetShaderResource(gRenderer->sunLightShadows, 2);
		gRenderer->SetShaderResource(scene->preTranslucentBuff, 3);

		IShader* overridePsShader = nullptr;
		if (cvRenderMaterialMode.AsInt() == 1 || scene->lights.Size() == 0)
			overridePsShader = gRenderer->debugUnlit->psShader;
		else if (cvRenderMaterialMode.AsInt() == 2)
			overridePsShader = gRenderer->debugNormalForward->psShader;

		if (overridePsShader)
			gRenderer->SetPsShader(overridePsShader);

		for (auto& rc : forwardMeshes)
		{
			//FObjectInfoBuffer objectInfo{ *rc->skeletonMatrices.Data(), rc->transform, FVector(), 0 };
			FObjectInfoBuffer objectInfo;
			objectInfo.transform = rc.Value->transform;
			objectInfo.position = rc.Key->GetPosition();
			memcpy(objectInfo.skeletonMatrices, rc.Value->skeletonMatrices.Data(), FMath::Min(rc.Value->skeletonMatrices.Size(), 48ull) * sizeof(FMatrix));
			gRenderer->objectBuffer->Update(sizeof(FObjectInfoBuffer), &objectInfo);

			FForwardLightsBuffer lights{};
			CreateForwardLightBuffer(objectInfo.position, scene->GetLights(), lights);
			gRenderer->forwardLightsBuffer->Update(sizeof(FForwardLightsBuffer), &lights);

			gRenderer->SetVsShader(rc.Value->mat->GetVsShader());
			if (!overridePsShader)
				gRenderer->SetPsShader(rc.Value->mat->GetPsShader());

			gRenderer->DrawMesh(rc.Value);
		}

		for (auto& rc : curCommands)
		{
			if (rc.type != FRenderCommand::DRAW_MESH)
				continue;

			FObjectInfoBuffer objectInfo{ { FMatrix(1.f) }, rc.drawMesh.transform, FVector(), 0 };
			gRenderer->objectBuffer->Update(sizeof(FObjectInfoBuffer), &objectInfo);

			gRenderer->SetVsShader(rc.drawMesh.material->GetVsShader());
			if (!overridePsShader)
				gRenderer->SetPsShader(rc.drawMesh.material->GetPsShader());

			gRenderer->DrawMesh(&rc.drawMesh);
		}

		UnlockGPU();
	}

	Blit(scene->colorBuffer, scene->preTranslucentBuff);
	gRenderer->SetViewport(0.f, 0.f, (float)viewWidth, (float)viewHeight);

	// ------------- FORWARD TRANSPARENT PASS -------------
	{
		curCommands.Clear();
		GetRenderCommands(R_TRANSPARENT_PASS, scene->renderQueue, curCommands);

		static TArray<TPair<CPrimitiveProxy*, FMeshBuilder::FRenderMesh*>> forwardMeshes;
		forwardMeshes.Clear();
		GetMeshesToDraw(R_TRANSPARENT_PASS, dynamicMeshes, forwardMeshes);

		LockGPU();
		gRenderer->SetBlendMode(EBlendMode::BLEND_ADDITIVE);

		gRenderer->SetFrameBuffer(scene->colorBuffer, scene->depth);
		gRenderer->SetShaderBuffer(gRenderer->sceneBuffer, 1);
		gRenderer->SetShaderBuffer(gRenderer->forwardLightsBuffer, 2);
		gRenderer->SetShaderBuffer(gRenderer->objectBuffer, 3);

		gRenderer->SetShaderResource(gRenderer->sunLightShadows, 2);
		gRenderer->SetShaderResource(scene->preTranslucentBuff, 3);

		IShader* overridePsShader = nullptr;
		if (cvRenderMaterialMode.AsInt() == 1 || scene->lights.Size() == 0)
			overridePsShader = gRenderer->debugUnlit->psShader;
		else if (cvRenderMaterialMode.AsInt() == 2)
			overridePsShader = gRenderer->debugNormalForward->psShader;

		if (overridePsShader)
			gRenderer->SetPsShader(overridePsShader);

		static std::vector<std::pair<float, uint64>> sortedDraws;
		sortedDraws.clear();

		for (int i = 0; i < curCommands.Size(); i++)
		{
			if (curCommands[i].type != FRenderCommand::DRAW_MESH)
				continue;

			//uint64 bType = 0;
			FVector pos = curCommands[i].drawMesh.transform.GetPosition();
			float distance = FVector::Distance(pos, camera->position);

			uint64 index{};
			((uint32*)(&index))[0] = i;
			((uint32*)(&index))[1] = 0;

			sortedDraws.push_back({ distance, index });
		}
		for (int i = 0; i < forwardMeshes.Size(); i++)
		{
			FVector pos = forwardMeshes[i].Key->GetPosition();
			float distance = FVector::Distance(pos, camera->position);

			uint64 bType = 1;
			uint64 index{};
			((uint32*)(&index))[0] = i;
			((uint32*)(&index))[1] = bType;

			sortedDraws.push_back({ distance, index });
		}
		// Sort draw calls.
		std::sort(sortedDraws.begin(), sortedDraws.end(), [](std::pair<float, uint64> a, std::pair<float, uint64> b) { return a.first > b.first; });

		for (auto& draw : sortedDraws)
		{
			int i = ((uint32*)(&draw.second))[0];
			int type = ((uint32*)(&draw.second))[1];

			if (type == 1)
			{
				TPair<CPrimitiveProxy*, FMeshBuilder::FRenderMesh*>& rc = forwardMeshes[i];

				//FObjectInfoBuffer objectInfo{ *rc->skeletonMatrices.Data(), rc->transform, FVector(), 0 };
				FObjectInfoBuffer objectInfo;
				objectInfo.transform = rc.Value->transform;
				objectInfo.position = rc.Key->GetPosition();
				memcpy(objectInfo.skeletonMatrices, rc.Value->skeletonMatrices.Data(), FMath::Min(rc.Value->skeletonMatrices.Size(), 48ull) * sizeof(FMatrix));
				gRenderer->objectBuffer->Update(sizeof(FObjectInfoBuffer), &objectInfo);

				FForwardLightsBuffer lights{};
				CreateForwardLightBuffer(objectInfo.position, scene->GetLights(), lights);
				gRenderer->forwardLightsBuffer->Update(sizeof(FForwardLightsBuffer), &lights);

				gRenderer->SetVsShader(rc.Value->mat->GetVsShader());
				if (!overridePsShader)
					gRenderer->SetPsShader(rc.Value->mat->GetPsShader());

				gRenderer->DrawMesh(rc.Value);
			}
			else
			{
				FRenderCommand& rc = curCommands[i];
				if (rc.type != FRenderCommand::DRAW_MESH)
					continue;

				FObjectInfoBuffer objectInfo{ { FMatrix(1.f) }, rc.drawMesh.transform, FVector(), 0 };
				gRenderer->objectBuffer->Update(sizeof(FObjectInfoBuffer), &objectInfo);

				gRenderer->SetVsShader(rc.drawMesh.material->GetVsShader());
				if (!overridePsShader)
					gRenderer->SetPsShader(rc.drawMesh.material->GetPsShader());

				gRenderer->DrawMesh(&rc.drawMesh);
			}
		}

		UnlockGPU();
	}

	// ------------- DEBUG PASS -------------
	for (int pass = 0; pass < 2; pass++)
	{
		curCommands.Clear();
		GetRenderCommands(pass == 0 ? R_DEBUG_PASS : R_DEBUG_OVERLAY_PASS, scene->renderQueue, curCommands);

		static TArray<TPair<CPrimitiveProxy*, FMeshBuilder::FRenderMesh*>> debugMeshes;
		debugMeshes.Clear();
		GetMeshesToDraw(pass == 0 ? R_DEBUG_PASS : R_DEBUG_OVERLAY_PASS, dynamicMeshes, debugMeshes);

		LockGPU();
		gRenderer->SetBlendMode(EBlendMode::BLEND_ADDITIVE);

		if (pass == 1)
			scene->depth->Clear();

		gRenderer->SetFrameBuffer(scene->colorBuffer, scene->depth);
		gRenderer->SetShaderBuffer(gRenderer->sceneBuffer, 1);
		gRenderer->SetShaderBuffer(gRenderer->objectBuffer, 3);

		static std::vector<std::pair<float, uint64>> sortedDraws;
		sortedDraws.clear();

		for (int i = 0; i < curCommands.Size(); i++)
		{
			if (curCommands[i].type != FRenderCommand::DRAW_MESH)
				continue;

			//uint64 bType = 0;
			FVector pos = curCommands[i].drawMesh.transform.GetPosition();
			float distance = FVector::Distance(pos, camera->position);
			
			uint64 index{};
			((uint32*)(&index))[0] = i;
			((uint32*)(&index))[1] = 0;

			sortedDraws.push_back({ distance, index });
		}
		for (int i = 0; i < debugMeshes.Size(); i++)
		{
			FVector pos = debugMeshes[i].Key->GetPosition();
			float distance = FVector::Distance(pos, camera->position);

			uint64 bType = 1;
			uint64 index{};
			((uint32*)(&index))[0] = i;
			((uint32*)(&index))[1] = bType;

			sortedDraws.push_back({ distance, index });
		}
		// Sort draw calls.
		std::sort(sortedDraws.begin(), sortedDraws.end(), [](std::pair<float, uint64> a, std::pair<float, uint64> b) { return a.first > b.first; });

		for (auto& draw : sortedDraws)
		{
			int i = ((uint32*)(&draw.second))[0];
			int type = ((uint32*)(&draw.second))[1];

			if (type == 0)
			{
				FRenderCommand& rc = curCommands[i];
				if (rc.type != FRenderCommand::DRAW_MESH)
					continue;

				FObjectInfoBuffer objectInfo{ { FMatrix(1.f) }, rc.drawMesh.transform, FVector(), 0 };
				gRenderer->objectBuffer->Update(sizeof(FObjectInfoBuffer), &objectInfo);

				gRenderer->SetVsShader(rc.drawMesh.material->GetVsShader());
				gRenderer->SetPsShader(rc.drawMesh.material->GetPsShader());
				gRenderer->DrawMesh(&rc.drawMesh);
			}

			if (type == 1)
			{
				TPair<CPrimitiveProxy*, FMeshBuilder::FRenderMesh*>& rc = debugMeshes[i];

				FObjectInfoBuffer objectInfo;
				objectInfo.transform = rc.Value->transform;
				memcpy(objectInfo.skeletonMatrices, rc.Value->skeletonMatrices.Data(), FMath::Min(rc.Value->skeletonMatrices.Size(), 48ull));
				gRenderer->objectBuffer->Update(sizeof(FObjectInfoBuffer), &objectInfo);

				gRenderer->SetVsShader(rc.Value->mat->GetVsShader());
				gRenderer->SetPsShader(rc.Value->mat->GetPsShader());
				gRenderer->DrawMesh(rc.Value);
			}
		}

		UnlockGPU();
	}
	Blit(scene->colorBuffer, scene->frameBuffer);
}

void IRenderer::RenderShadowMaps(CRenderScene* scene)
{
	auto& lights = scene->GetLights();
	if (lights.Size() == 0 || !cvRenderShadowEnabled.AsBool())
		return;
	
	CLightProxy* sunLight = nullptr;
	static TArray<CLightProxy*> spotLights;
	static TArray<CLightProxy*> pointLights;
	spotLights.Clear();
	pointLights.Clear();

	static TArray<FRenderCommand> curCommands;
	static TArray<TPair<CPrimitiveProxy*, FMeshBuilder::FRenderMesh>> drawMeshes;
	drawMeshes.Clear();
	curCommands.Clear();

	GetRenderCommands(R_DEFERRED_PASS, scene->renderQueue, curCommands);
	GetRenderCommands(R_FORWARD_PASS, scene->renderQueue, curCommands);

	for (CLightProxy* light : lights)
	{
		light->shadowIndex = -1;
		if (!light->CastShadows())
			continue;

		if (light->bakingMode > CLightProxy::BAKE_INDIRECT)
			continue;

		if (light->type == CLightProxy::DIRECTIONAL_LIGHT && !sunLight)
		{
			sunLight = light;
		}
		else if (light->type == CLightProxy::SPOT_LIGHT)
		{
			spotLights.Add(light);
		}
		else if (light->type == CLightProxy::POINT_LIGHT)
		{
			pointLights.Add(light);
		}
	}

	FShadowDataBuffer shadowData{};

	for (auto* primitive : scene->primitves)
	{
		if (!primitive->IsVisible() || !primitive->CastShadows())
			continue;

		//dynamicMeshes.Add({ primitive, FMeshBuilder() });
		//FMeshBuilder& dynamic = dynamicMeshes.last()->Value;
		FMeshBuilder dynamic;
		primitive->GetDynamicMeshes(dynamic);

		for (int i = 0; i < dynamic.GetMeshes().Size(); i++)
		{
			const FMeshBuilder::FRenderMesh& mesh = dynamic.GetMeshes()[i];
			if (mesh.rp != R_FORWARD_PASS && mesh.rp != R_DEFERRED_PASS)
				continue;

			drawMeshes.Add({ primitive, mesh });
		}
	}

	if (scene->primaryCamera && sunLight)
	{
		sunLight->shadowIndex = 0;

		// Update sun light camera position
		FVector camPos = scene->primaryCamera->position;
		camPos += scene->primaryCamera->GetForwardVector() * 2;

		if (FVector::Distance(camPos, scene->sunLightCamPos) > 0.15f)
		{
			scene->sunLightCamPos = camPos;
			scene->sunLightCamDir = scene->primaryCamera->GetForwardVector();
		}

		gRenderer->sunLightShadows->Clear();

		// Render directional light shadows
		gRenderer->SetFrameBuffer(nullptr, gRenderer->sunLightShadows);

		for (int j = 0; j < 4; j++)
		{
			gRenderer->SetViewport(gRenderer->shadowTexSize * j, 0, gRenderer->shadowTexSize, gRenderer->shadowTexSize);

			float fDistance = 3 * ((j + 1) * (j + 1));
			FVector camTarget = scene->sunLightCamPos + (scene->sunLightCamDir * (fDistance / 2));
			//FVector shadowCamPos = camTarget + (sunLight->direction * (150 + fDistance));
			FVector shadowCamPos = camTarget + (sunLight->direction * 350);

			FMatrix shadowProjection = FMatrix::Orthographic(-fDistance, fDistance, -fDistance, fDistance, 1.f, 500.f);
			//FMatrix shadowView = FMatrix::LookAt(shadowCamPos, camTarget, FVector(0, 1, 0));
			FMatrix shadowView = FMatrix(1.f).Translate(shadowCamPos) * sunLight->rotation;
			shadowView = shadowView.Inverse();
			FMatrix shadowMatrix = shadowProjection * shadowView;

			shadowData.vSunShadowMatrix[j] = shadowMatrix;
			shadowData.vSunShadowBias = sunLight->shadowBias;

			FSceneInfoBuffer sceneInfo{ shadowMatrix,
				shadowView, shadowProjection,
				shadowCamPos, 0u, sunLight->direction, 0u, scene->GetTime() };
			gRenderer->sceneBuffer->Update(sizeof(FSceneInfoBuffer), &sceneInfo);

			for (auto& rc : drawMeshes)
			{
				FObjectInfoBuffer objectInfo;
				objectInfo.transform = rc.Value.transform;
				objectInfo.position = rc.Key->GetPosition();
				memcpy(objectInfo.skeletonMatrices, rc.Value.skeletonMatrices.Data(), FMath::Min(rc.Value.skeletonMatrices.Size(), 48ull) * sizeof(FMatrix));
				gRenderer->objectBuffer->Update(sizeof(FObjectInfoBuffer), &objectInfo);

				gRenderer->SetVsShader(rc.Value.mat->GetVsShader());
				gRenderer->SetPsShader(nullptr);

				gRenderer->DrawMesh(&rc.Value);
			}

			for (auto& rc : curCommands)
			{
				if (rc.type != FRenderCommand::DRAW_MESH)
					continue;

				FObjectInfoBuffer objectInfo{ { FMatrix(1.f) }, rc.drawMesh.transform, FVector(), 0 };
				gRenderer->objectBuffer->Update(sizeof(FObjectInfoBuffer), &objectInfo);

				gRenderer->SetVsShader(rc.drawMesh.material->GetVsShader());
				gRenderer->SetPsShader(nullptr);

				gRenderer->DrawMesh(&rc.drawMesh);
			}
		}
	}

	gRenderer->forwardShadowDataBuffer->Update(sizeof(FShadowDataBuffer), &shadowData);
}

void IRenderer::RenderUserInterface(CRenderScene* scene)
{

}
