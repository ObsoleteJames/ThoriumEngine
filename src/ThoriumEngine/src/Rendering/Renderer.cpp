
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

CConVar cvRenderFBPointFilter("r.framebuffer.pointfilter", "config/graphics.cfg", 0, 0, 1);

CConVar cvForceForwardRendering("r.forceforward", "config/graphics.cfg", 0, 0, 1);

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

	debugUnlit = CShaderSource::GetShaderSource("Unlit");
	debugUnlit->LoadShaderObjects();

	debugNormalForward = CShaderSource::GetShaderSource("DebugNormalForward");
	debugNormalForward->LoadShaderObjects();

	shaderScreenPlane = CShaderSource::GetShaderSource("ScreenPlaneVS");
	shaderScreenPlane->LoadShaderObjects();
	
	shaderBlit = CShaderSource::GetShaderSource("blitFrameBuffer");
	shaderBlit->LoadShaderObjects();

	shaderDeferredDirLight = CShaderSource::GetShaderSource("DeferredDirectionalLight");
	shaderDeferredDirLight->LoadShaderObjects();

	shaderDeferredPointLight = CShaderSource::GetShaderSource("DeferredPointLight");
	shaderDeferredPointLight->LoadShaderObjects();

	shaderPPExposure = CShaderSource::GetShaderSource("PPExposure");
	shaderPPExposure->LoadShaderObjects();

	sceneBuffer = gRenderer->CreateShaderBuffer(nullptr, sizeof(FSceneInfoBuffer));
	objectBuffer = gRenderer->CreateShaderBuffer(nullptr, sizeof(FObjectInfoBuffer));
	forwardLightsBuffer = gRenderer->CreateShaderBuffer(nullptr, sizeof(FForwardLightsBuffer));
	shadowDataBuffer = gRenderer->CreateShaderBuffer(nullptr, sizeof(FShadowDataBuffer));

	deferredLightBuffer = gRenderer->CreateShaderBuffer(nullptr, sizeof(FSpotLightData));

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

	meshIcoSphere = CResourceManager::GetResource<CModelAsset>("models/IcoSphere.thmdl");
	meshIcoSphere->Load(0);

	gDebugRenderer = new CDebugRenderer();
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
		blitDataBuffer = gRenderer->CreateShaderBuffer(nullptr, sizeof(float) * 4);
	
	blitDataBuffer->Update(sizeof(float) * 4, sizeScalar);

	gRenderer->SetViewport(0, 0, (float)widthB, (float)heightB);
	gRenderer->SetFrameBuffer(b);
	gRenderer->SetShaderResource(a, 1);
	gRenderer->SetShaderBuffer(blitDataBuffer, 0);
	gRenderer->SetVsShader(gRenderer->shaderScreenPlane->GetShader(ShaderType_Vertex));
	gRenderer->SetPsShader(gRenderer->shaderBlit->GetShader(ShaderType_Fragment));

	FMesh mesh;
	mesh.numVertices = 3;
	gRenderer->DrawMesh(&mesh);
}

void IRenderer::Blit(IFrameBuffer* a, IFrameBuffer* b, FVector2 viewportPos, FVector2 viewportScale)
{
	int w, h;
	a->GetSize(w, h);

	int widthB, heightB;
	b->GetSize(widthB, heightB);

	float sizeScalar[4] = { viewportPos.x, viewportPos.y, viewportScale.x, viewportScale.y };

	if (!blitDataBuffer)
		blitDataBuffer = gRenderer->CreateShaderBuffer(nullptr, sizeof(float) * 4);

	blitDataBuffer->Update(sizeof(float) * 4, sizeScalar);

	gRenderer->SetViewport(0, 0, (float)widthB, (float)heightB);
	gRenderer->SetFrameBuffer(b);
	gRenderer->SetShaderResource(a, 1);
	gRenderer->SetShaderBuffer(blitDataBuffer, 0);
	gRenderer->SetVsShader(gRenderer->shaderScreenPlane->GetShader(ShaderType_Vertex));
	gRenderer->SetPsShader(gRenderer->shaderBlit->GetShader(ShaderType_Fragment));

	FMesh mesh;
	mesh.numVertices = 3;
	gRenderer->DrawMesh(&mesh);
}

void IRenderer::renderAll()
{
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

void IRenderer::PreDeferredLightSetup(CRenderScene* scene)
{
	gRenderer->SetVsShader(gRenderer->shaderScreenPlane->GetShader(ShaderType_Vertex));

	gRenderer->SetFrameBuffer(scene->colorBuffer);

	//gRenderer->SetShaderResource(scene->preTranslucentBuff, 3);
	gRenderer->SetShaderResource(scene->GBufferA, 4);
	gRenderer->SetShaderResource(scene->GBufferB, 5);
	gRenderer->SetShaderResource(scene->GBufferC, 6);
	gRenderer->SetShaderResource(scene->GBufferD, 7);
	gRenderer->SetShaderResource(scene->depthTex, 8);

	gRenderer->SetShaderBuffer(gRenderer->sceneBuffer, 1);
	gRenderer->SetShaderBuffer(gRenderer->deferredLightBuffer, 2);
	gRenderer->SetShaderBuffer(gRenderer->shadowDataBuffer, 4);
}

void IRenderer::RenderCamera(CRenderScene* scene, CCameraProxy* camera)
{
	static TArray<FRenderCommand> curCommands;
	SizeType queueLastIndex = 0;

	IFrameBuffer* renderTarget = camera->renderTarget ? camera->renderTarget : scene->frameBuffer;

	gRenderer->curCamera = camera;

	int viewWidth, viewHeight;
	renderTarget->GetSize(viewWidth, viewHeight);
	renderTarget->Clear();

	float sp = scene->ScreenPercentage() / 100.f;
	
	if (scene->depth)
		scene->depth->Clear();

	scene->colorBuffer->Clear();

	if (viewWidth > scene->GetFrameBufferWidth() || viewHeight > scene->GetFrameBufferHeight())
		scene->ResizeBuffers(viewWidth, viewHeight);

	gRenderer->SetViewport(0.f, 0.f, (float)viewWidth * sp, (float)viewHeight * sp);

	camera->CalculateMatrix((float)viewWidth / (float)viewHeight);

	FMatrix camMatrix = camera->projection * camera->view;
	float exposure = 1.0f;
	float gamma = 1.6f;

	CPostProcessVolumeProxy* ppVolumeA = nullptr;
	CPostProcessVolumeProxy* ppVolumeB = nullptr; // for if we have a volume with fading.

	// figure out which volume has the highest priority
	for (auto& volume : scene->ppVolumes)
	{
		if (volume->postProcessMaterial)
			continue;

		if (volume->IsGlobal())
		{
			if (!ppVolumeA)
				ppVolumeA = volume;
			else if (volume->GetPriority() > ppVolumeA->GetPriority())
				ppVolumeA = volume;
			continue;
		}

		FVector relCamPos = volume->rotation.Rotate(camera->position - volume->Bounds().position) + volume->Bounds().position;
		if (volume->Bounds().IsInside(relCamPos))
		{
			float influence = volume->GetInfluence(camera);
			if (influence < 1.f && ppVolumeA)
			{
				if (!ppVolumeB)
					ppVolumeB = volume;
				else if (volume->GetPriority() > ppVolumeB->GetPriority())
					ppVolumeB = volume;
				continue;
			}

			if (!ppVolumeA)
				ppVolumeA = volume;
			else if (volume->GetPriority() > ppVolumeA->GetPriority())
				ppVolumeA = volume;
			continue;
		}
	}

	if (ppVolumeA)
	{
		exposure = ppVolumeA->PostProcessSettings().exposureIntensity;
		if (ppVolumeB)
			exposure = FMath::Lerp(exposure, ppVolumeB->PostProcessSettings().exposureIntensity, ppVolumeB->GetInfluence(camera));

		gamma = ppVolumeA->PostProcessSettings().gamma;
		if (ppVolumeB)
			gamma = FMath::Lerp(gamma, ppVolumeB->PostProcessSettings().gamma, ppVolumeB->GetInfluence(camera));
	}

	FSceneInfoBuffer sceneInfo{ camMatrix, camera->view, camera->projection,
		camMatrix.Inverse(), camera->view.Inverse(), camera->projection.Inverse(),
		camera->position, 0u, camera->GetForwardVector(), 0u, scene->GetTime(), exposure, gamma, 0, FVector2((float)viewWidth, (float)viewHeight) / FVector2((float)scene->bufferWidth, (float)scene->bufferHeight) };
	gRenderer->sceneBuffer->Update(sizeof(FSceneInfoBuffer), &sceneInfo);

	static TArray<TPair<CPrimitiveProxy*, FMeshBuilder>> dynamicMeshes;
	dynamicMeshes.Clear();

	for (auto* primitive : scene->primitives)
	{
		if (!primitive->IsVisible())
			continue;

		if (!primitive->DoFrustumCull(sceneInfo.camMatrix))
			continue;

		dynamicMeshes.Add({ primitive, FMeshBuilder() });
		FMeshBuilder& dynamic = dynamicMeshes.last()->Value;

		primitive->GetDynamicMeshes(dynamic);
		gRenderStats.drawPrimitives++;
	}

	// List of all meshes to be drawn during a pass.
	static TArray<TPair<CPrimitiveProxy*, FMeshBuilder::FRenderMesh*>> curMeshes;

#if ENABLE_DEFERRED_RENDERING
	// ------------- DEFERRED PASS -------------
	if (cvForceForwardRendering.AsBool() == false)
	{
		curCommands.Clear();
		GetRenderCommands(R_DEFERRED_PASS, scene->renderQueue, curCommands);

		curMeshes.Clear();
		GetMeshesToDraw(R_DEFERRED_PASS, dynamicMeshes, curMeshes);

		gRenderer->SetBlendMode(EBlendMode::BLEND_DISABLED);

		LockGPU();

		scene->GBufferA->Clear();
		scene->GBufferB->Clear();
		scene->GBufferC->Clear();
		scene->GBufferD->Clear();
		
		IFrameBuffer* fbList[] = {
			scene->colorBuffer,
			scene->GBufferA,
			scene->GBufferB,
			scene->GBufferC,
			scene->GBufferD,
		};

		gRenderer->SetViewport(0.f, 0.f, (float)viewWidth * sp, (float)viewHeight * sp);
		gRenderer->SetFrameBuffers(fbList, 5, scene->depth);

		gRenderer->SetShaderBuffer(gRenderer->sceneBuffer, 1);
		gRenderer->SetShaderBuffer(gRenderer->objectBuffer, 3);

		gRenderer->SetShaderResource(scene->preTranslucentBuff, 3);

		IShader* curVsShader = nullptr;
		IShader* curPsShader = nullptr;

		for (auto& rc : curMeshes)
		{
			FObjectInfoBuffer objectInfo;
			objectInfo.transform = rc.Value->transform;
			objectInfo.position = rc.Key->GetPosition();
			memcpy(objectInfo.skeletonMatrices, rc.Value->skeletonMatrices.Data(), FMath::Min(rc.Value->skeletonMatrices.Size(), 48ull) * sizeof(FMatrix));
			gRenderer->objectBuffer->Update(sizeof(FObjectInfoBuffer), &objectInfo);

			IShader* _shader = rc.Value->mat->GetVsShader(ShaderType_DeferredPass);
			if (_shader != curVsShader)
			{
				curVsShader = _shader;
				gRenderer->SetVsShader(curVsShader);
			}
			_shader = rc.Value->mat->GetPsShader(ShaderType_DeferredPass);
			if (_shader != curPsShader)
			{
				curPsShader = _shader;
				gRenderer->SetPsShader(curPsShader);
			}

			gRenderer->DrawMesh(rc.Value);
		}

		for (auto& rc : curCommands)
		{
			if (rc.type != FRenderCommand::DRAW_MESH)
				continue;

			FObjectInfoBuffer objectInfo{ { FMatrix(1.f) }, rc.drawMesh.transform, FVector(), 0 };
			gRenderer->objectBuffer->Update(sizeof(FObjectInfoBuffer), &objectInfo);

			gRenderer->SetVsShader(rc.drawMesh.material->GetVsShader(ShaderType_DeferredPass));
			gRenderer->SetPsShader(rc.drawMesh.material->GetPsShader(ShaderType_DeferredPass));

			gRenderer->DrawMesh(&rc.drawMesh);
		}

		PreDeferredLightSetup(scene);

		gRenderer->SetBlendMode(EBlendMode::BLEND_ADDITIVE_COLOR);

		gRenderer->CopyResource(scene->depth, scene->depthTex);

		// -- Directional Lights --
		gRenderer->SetPsShader(gRenderer->shaderDeferredDirLight->GetShader(ShaderType_Fragment));
		for (auto& light : scene->GetLights())
		{
			if (!light->Enabled())
				continue;

			if (light->type == CLightProxy::DIRECTIONAL_LIGHT)
			{
				FDirectionalLightData data;
				data.color = light->color;
				data.direction = light->direction;
				data.intensity = light->intensity;
				data.shadowIndex = light->shadowIndex;

				gRenderer->deferredLightBuffer->Update(sizeof(data), &data);
				gRenderer->SetShaderResource(gRenderer->sunLightShadows, 2);

				FMesh mesh;
				mesh.numVertices = 3;
				gRenderer->DrawMesh(&mesh);
			}
		}

		// -- Point Lights --
		gRenderer->SetVsShader(gRenderer->shaderDeferredPointLight->GetShader(ShaderType_Vertex));
		gRenderer->SetPsShader(gRenderer->shaderDeferredPointLight->GetShader(ShaderType_Fragment));
		gRenderer->SetShaderBuffer(gRenderer->objectBuffer, 3);
		gRenderer->SetFaceCulling(false);

		for (auto& light : scene->GetLights())
		{
			if (!light->Enabled())
				continue;

			if (light->type == CLightProxy::POINT_LIGHT)
			{
				FPointLightData data;
				data.color = light->color;
				data.intensity = light->intensity;
				data.position = light->position;
				data.range = light->range;
				data.shadowIndex = light->shadowIndex;

				gRenderer->deferredLightBuffer->Update(sizeof(data), &data);

				FObjectInfoBuffer objectInfo;
				objectInfo.position = light->position;
				objectInfo.transform = FMatrix(1.f).Translate(data.position).Scale((light->range * 2) + 0.15f);
				gRenderer->objectBuffer->Update(sizeof(FObjectInfoBuffer), &objectInfo);
				
				const FMesh& mesh = gRenderer->meshIcoSphere->GetMeshes()[0];
				gRenderer->DrawMesh((FMesh*)&mesh);
			}
		}

		UnlockGPU();
	}
#endif

	gRenderer->SetFaceCulling(true);
	gRenderer->SetBlendMode(EBlendMode::BLEND_DISABLED);

	{
		int matMode = cvRenderMaterialMode.AsInt();
		if (matMode == 1)
			Blit(scene->GBufferC, scene->colorBuffer);
		if (matMode == 2)
			Blit(scene->GBufferA, scene->colorBuffer);
	}

	//gRenderer->CopyResource(scene->colorBuffer, scene->preTranslucentBuff);
	gRenderer->SetViewport(0.f, 0.f, (float)viewWidth * sp, (float)viewHeight * sp);

	// ------------- FORWARD PASS -------------
	{
		curCommands.Clear();
		GetRenderCommands(R_FORWARD_PASS, scene->renderQueue, curCommands);

		curMeshes.Clear();
		GetMeshesToDraw(R_FORWARD_PASS, dynamicMeshes, curMeshes);

		LockGPU();
		gRenderer->SetBlendMode(EBlendMode::BLEND_DISABLED);

		gRenderer->SetFrameBuffer(scene->colorBuffer, scene->depth);
		gRenderer->SetShaderBuffer(gRenderer->sceneBuffer, 1);
		gRenderer->SetShaderBuffer(gRenderer->forwardLightsBuffer, 2);
		gRenderer->SetShaderBuffer(gRenderer->objectBuffer, 3);
		gRenderer->SetShaderBuffer(gRenderer->shadowDataBuffer, 4);
		
		gRenderer->SetShaderResource(gRenderer->sunLightShadows, 2);
		gRenderer->SetShaderResource(scene->preTranslucentBuff, 3);

		IShader* overridePsShader = nullptr;
		if (cvRenderMaterialMode.AsInt() == 1 || scene->lights.Size() == 0)
			overridePsShader = gRenderer->debugUnlit->GetShader(ShaderType_Fragment);
		else if (cvRenderMaterialMode.AsInt() == 2)
			overridePsShader = gRenderer->debugNormalForward->GetShader(ShaderType_Fragment);

		if (overridePsShader)
			gRenderer->SetPsShader(overridePsShader);

		for (auto& rc : curMeshes)
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

			gRenderer->SetVsShader(rc.Value->mat->GetVsShader(ShaderType_ForwardPass));
			if (!overridePsShader)
				gRenderer->SetPsShader(rc.Value->mat->GetPsShader(ShaderType_ForwardPass));

			gRenderer->DrawMesh(rc.Value);
		}

		for (auto& rc : curCommands)
		{
			if (rc.type != FRenderCommand::DRAW_MESH)
				continue;

			FObjectInfoBuffer objectInfo{ { FMatrix(1.f) }, rc.drawMesh.transform, FVector(), 0 };
			gRenderer->objectBuffer->Update(sizeof(FObjectInfoBuffer), &objectInfo);

			gRenderer->SetVsShader(rc.drawMesh.material->GetVsShader(ShaderType_ForwardPass));
			if (!overridePsShader)
				gRenderer->SetPsShader(rc.drawMesh.material->GetPsShader(ShaderType_ForwardPass));

			gRenderer->DrawMesh(&rc.drawMesh);
		}

		UnlockGPU();
	}

	gRenderer->CopyResource(scene->colorBuffer, scene->preTranslucentBuff);
	gRenderer->SetViewport(0.f, 0.f, (float)viewWidth * sp, (float)viewHeight * sp);

	// ------------- FORWARD TRANSPARENT PASS -------------
	{
		curCommands.Clear();
		GetRenderCommands(R_TRANSPARENT_PASS, scene->renderQueue, curCommands);

		curMeshes.Clear();
		GetMeshesToDraw(R_TRANSPARENT_PASS, dynamicMeshes, curMeshes);

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
			overridePsShader = gRenderer->debugUnlit->GetShader(ShaderType_ForwardPass);
		else if (cvRenderMaterialMode.AsInt() == 2)
			overridePsShader = gRenderer->debugNormalForward->GetShader(ShaderType_ForwardPass);

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
		for (int i = 0; i < curMeshes.Size(); i++)
		{
			FVector pos = curMeshes[i].Key->GetPosition();
			float distance = FVector::Distance(pos, camera->position);

			uint64 bType = 1;
			uint64 index{};
			((uint32*)(&index))[0] = i;
			((uint32*)(&index))[1] = (uint32)bType;

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
				TPair<CPrimitiveProxy*, FMeshBuilder::FRenderMesh*>& rc = curMeshes[i];

				//FObjectInfoBuffer objectInfo{ *rc->skeletonMatrices.Data(), rc->transform, FVector(), 0 };
				FObjectInfoBuffer objectInfo;
				objectInfo.transform = rc.Value->transform;
				objectInfo.position = rc.Key->GetPosition();
				memcpy(objectInfo.skeletonMatrices, rc.Value->skeletonMatrices.Data(), FMath::Min(rc.Value->skeletonMatrices.Size(), 48ull) * sizeof(FMatrix));
				gRenderer->objectBuffer->Update(sizeof(FObjectInfoBuffer), &objectInfo);

				FForwardLightsBuffer lights{};
				CreateForwardLightBuffer(objectInfo.position, scene->GetLights(), lights);
				gRenderer->forwardLightsBuffer->Update(sizeof(FForwardLightsBuffer), &lights);

				gRenderer->SetVsShader(rc.Value->mat->GetVsShader(ShaderType_ForwardPass));
				if (!overridePsShader)
					gRenderer->SetPsShader(rc.Value->mat->GetPsShader(ShaderType_ForwardPass));

				gRenderer->DrawMesh(rc.Value);
			}
			else
			{
				FRenderCommand& rc = curCommands[i];
				if (rc.type != FRenderCommand::DRAW_MESH)
					continue;

				FObjectInfoBuffer objectInfo{ { FMatrix(1.f) }, rc.drawMesh.transform, FVector(), 0 };
				gRenderer->objectBuffer->Update(sizeof(FObjectInfoBuffer), &objectInfo);

				gRenderer->SetVsShader(rc.drawMesh.material->GetVsShader(ShaderType_ForwardPass));
				if (!overridePsShader)
					gRenderer->SetPsShader(rc.drawMesh.material->GetPsShader(ShaderType_ForwardPass));

				gRenderer->DrawMesh(&rc.drawMesh);
			}
		}

		UnlockGPU();
	}

	gRenderer->CopyResource(scene->colorBuffer, scene->preTranslucentBuff);

	// ------------- POST PROCESSING -------------
	{
		std::multimap<int, CPostProcessVolumeProxy*> postProcessMats;
	
		for (auto& volume : scene->ppVolumes)
		{
			if (!volume->postProcessMaterial)
				continue;

			if (!volume->IsEnabled())
				continue;

			if (volume->IsGlobal() || volume->GetInfluence(camera) == 1.f)
				postProcessMats.insert(std::pair(volume->priority, volume));
		}

		LockGPU();
		{
			// TODO: dynamic exposure
			gRenderer->SetBlendMode(EBlendMode::BLEND_DISABLED);
			gRenderer->SetFrameBuffer(scene->colorBuffer, nullptr);

			gRenderer->SetVsShader(gRenderer->shaderScreenPlane->GetShader(ShaderType_Vertex));
			gRenderer->SetPsShader(gRenderer->shaderPPExposure->GetShader(ShaderType_Fragment));
			
			gRenderer->SetShaderResource(scene->preTranslucentBuff, 0);

			FMesh mesh;
			mesh.numVertices = 3;
			gRenderer->DrawMesh(&mesh);
		}

		for (auto& v : postProcessMats)
		{
			auto volume = v.second;

			gRenderer->CopyResource(scene->colorBuffer, scene->preTranslucentBuff);

			gRenderer->SetFrameBuffer(scene->colorBuffer, nullptr);
			gRenderer->SetShaderResource(scene->preTranslucentBuff, 3);

			gRenderer->SetVsShader(gRenderer->shaderScreenPlane->GetShader(ShaderType_Vertex));
			gRenderer->SetPsShader(volume->postProcessMaterial->GetPsShader(0));

			gRenderer->SetMaterial(volume->postProcessMaterial);

			FMesh mesh;
			mesh.numVertices = 3;
			gRenderer->DrawMesh(&mesh);
		}
		UnlockGPU();
	}

	// ------------- DEBUG PASS -------------
	for (int pass = 0; pass < 2; pass++)
	{
		curCommands.Clear();
		GetRenderCommands(pass == 0 ? R_DEBUG_PASS : R_DEBUG_OVERLAY_PASS, scene->renderQueue, curCommands);

		curMeshes.Clear();
		GetMeshesToDraw(pass == 0 ? R_DEBUG_PASS : R_DEBUG_OVERLAY_PASS, dynamicMeshes, curMeshes);

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
		for (int i = 0; i < curMeshes.Size(); i++)
		{
			FVector pos = curMeshes[i].Key->GetPosition();
			float distance = FVector::Distance(pos, camera->position);

			uint64 bType = 1;
			uint64 index{};
			((uint32*)(&index))[0] = i;
			((uint32*)(&index))[1] = (uint32)bType;

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

				gRenderer->SetVsShader(rc.drawMesh.material->GetVsShader(0));
				gRenderer->SetPsShader(rc.drawMesh.material->GetPsShader(0));
				gRenderer->DrawMesh(&rc.drawMesh);
			}

			if (type == 1)
			{
				TPair<CPrimitiveProxy*, FMeshBuilder::FRenderMesh*>& rc = curMeshes[i];

				FObjectInfoBuffer objectInfo;
				objectInfo.transform = rc.Value->transform;
				memcpy(objectInfo.skeletonMatrices, rc.Value->skeletonMatrices.Data(), FMath::Min(rc.Value->skeletonMatrices.Size(), 48ull));
				gRenderer->objectBuffer->Update(sizeof(FObjectInfoBuffer), &objectInfo);

				gRenderer->SetVsShader(rc.Value->mat->GetVsShader(0));
				gRenderer->SetPsShader(rc.Value->mat->GetPsShader(0));
				gRenderer->DrawMesh(rc.Value);
			}
		}

		UnlockGPU();
	}
	Blit(scene->colorBuffer, renderTarget, FVector2(), FVector2((float)viewWidth / (float)scene->bufferWidth, (float)viewHeight / (float)scene->bufferHeight));
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

	for (auto* primitive : scene->primitives)
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
			gRenderer->SetViewport((float)(gRenderer->shadowTexSize * j), 0, (float)gRenderer->shadowTexSize, (float)gRenderer->shadowTexSize);

			float fDistance = (float)(3 * ((j + 1) * (j + 1)));
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

			FSceneInfoBuffer sceneInfo{ shadowMatrix, shadowView, shadowProjection,
				shadowMatrix.Inverse(), shadowView.Inverse(), shadowProjection.Inverse(),
				shadowCamPos, 0u, sunLight->direction, 0u, scene->GetTime() };
			gRenderer->sceneBuffer->Update(sizeof(FSceneInfoBuffer), &sceneInfo);

			gRenderer->SetShaderBuffer(gRenderer->sceneBuffer, 1);
			gRenderer->SetShaderBuffer(gRenderer->objectBuffer, 3);

			for (auto& rc : drawMeshes)
			{
				FObjectInfoBuffer objectInfo;
				objectInfo.transform = rc.Value.transform;
				objectInfo.position = rc.Key->GetPosition();
				memcpy(objectInfo.skeletonMatrices, rc.Value.skeletonMatrices.Data(), FMath::Min(rc.Value.skeletonMatrices.Size(), 48ull) * sizeof(FMatrix));
				gRenderer->objectBuffer->Update(sizeof(FObjectInfoBuffer), &objectInfo);

				gRenderer->SetVsShader(rc.Value.mat->GetVsShader(0));
				gRenderer->SetPsShader(nullptr);

				gRenderer->DrawMesh(&rc.Value);
			}

			for (auto& rc : curCommands)
			{
				if (rc.type != FRenderCommand::DRAW_MESH)
					continue;

				FObjectInfoBuffer objectInfo{ { FMatrix(1.f) }, rc.drawMesh.transform, FVector(), 0 };
				gRenderer->objectBuffer->Update(sizeof(FObjectInfoBuffer), &objectInfo);

				gRenderer->SetVsShader(rc.drawMesh.material->GetVsShader(0));
				gRenderer->SetPsShader(nullptr);

				gRenderer->DrawMesh(&rc.drawMesh);
			}
		}
	}

	gRenderer->shadowDataBuffer->Update(sizeof(FShadowDataBuffer), &shadowData);
}

void IRenderer::RenderUserInterface(CRenderScene* scene)
{

}
