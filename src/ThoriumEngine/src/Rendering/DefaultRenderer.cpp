
#include "DefaultRenderer.h"
#include "GraphicsInterface.h"
#include "RenderScene.h"
#include "Assets/Material.h"
#include "Game/Components/CameraComponent.h"
#include "Console.h"
#include "DebugRenderer.h"
#include "PostProcessing.h"

#include <map>
#include <algorithm>

static FPostProcessSettings defaultPostProcess;

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

void CDefaultRenderer::Init()
{
	IRenderer::Init();

	shaderScreenPlane = CShaderSource::GetShaderSource("ScreenPlaneVS");
	if (shaderScreenPlane)
		shaderScreenPlane->LoadShaderObjects();

	shaderBlit = CShaderSource::GetShaderSource("blitFrameBuffer");
	if (shaderBlit)
		shaderBlit->LoadShaderObjects();

	shaderDeferredDirLight = CShaderSource::GetShaderSource("DeferredDirectionalLight");
	if (shaderDeferredDirLight)
		shaderDeferredDirLight->LoadShaderObjects();

	shaderDeferredPointLight = CShaderSource::GetShaderSource("DeferredPointLight");
	if (shaderDeferredPointLight)
		shaderDeferredPointLight->LoadShaderObjects();

	shaderPPExposure = CShaderSource::GetShaderSource("PPExposure");
	if (shaderPPExposure)
		shaderPPExposure->LoadShaderObjects();

	shaderBloomPass = CShaderSource::GetShaderSource("BloomPass");
	if (shaderBloomPass)
		shaderBloomPass->LoadShaderObjects();

	shaderBloomPreFilter = CShaderSource::GetShaderSource("BloomPreFilter");
	if (shaderBloomPreFilter)
		shaderBloomPreFilter->LoadShaderObjects();

	shaderGaussianBlurV = CShaderSource::GetShaderSource("GaussianBlurV");
	if (shaderGaussianBlurV)
		shaderGaussianBlurV->LoadShaderObjects();
	shaderGaussianBlurH = CShaderSource::GetShaderSource("GaussianBlurH");
	if (shaderGaussianBlurH)
		shaderGaussianBlurH->LoadShaderObjects();

	sceneBuffer = gGHI->CreateShaderBuffer(nullptr, sizeof(FSceneInfoBuffer));
	objectBuffer = gGHI->CreateShaderBuffer(nullptr, sizeof(FObjectInfoBuffer));
	forwardLightsBuffer = gGHI->CreateShaderBuffer(nullptr, sizeof(FForwardLightsBuffer));
	shadowDataBuffer = gGHI->CreateShaderBuffer(nullptr, sizeof(FShadowDataBuffer));

	deferredLightBuffer = gGHI->CreateShaderBuffer(nullptr, sizeof(FSpotLightData));

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
		sunLightShadows = gGHI->CreateDepthBuffer(sunDepth);
		});

	FDepthBufferInfo sunDepth{};
	sunDepth.width = shadowTexSize * 4;
	sunDepth.height = shadowTexSize;
	sunDepth.bShaderResource = true;
	sunDepth.format = TH_DBF_R16;
	sunDepth.arraySize = 1;

	sunLightShadows = gGHI->CreateDepthBuffer(sunDepth);

	meshIcoSphere = CAssetManager::GetAsset<CModelAsset>("models/IcoSphere.thasset");
	if (meshIcoSphere)
		meshIcoSphere->Load(0);

	// check if *most* assets are present.
	bInitialized = meshIcoSphere && debugUnlit && debugNormalForward;

	if (bInitialized)
		gDebugRenderer = new CDebugRenderer();
	else
		CONSOLE_LogError("IRenderer", "Failed to initialize! rendering is disabled");
}

void CDefaultRenderer::PreDeferredLightSetup(CRenderScene* scene)
{
	gGHI->SetVsShader(shaderScreenPlane->GetShader(ShaderType_Vertex));

	gGHI->SetFrameBuffer(scene->colorBuffer);

	gGHI->SetShaderResource(scene->GBufferA, 4);
	gGHI->SetShaderResource(scene->GBufferB, 5);
	gGHI->SetShaderResource(scene->GBufferC, 6);
	gGHI->SetShaderResource(scene->GBufferD, 7);
	gGHI->SetShaderResource(scene->depthTex, 8);

	gGHI->SetShaderBuffer(sceneBuffer, 1);
	gGHI->SetShaderBuffer(deferredLightBuffer, 2);
	gGHI->SetShaderBuffer(shadowDataBuffer, 4);
}

void CDefaultRenderer::RenderCamera(CRenderScene* scene, CCameraProxy* camera)
{
	static TArray<FRenderCommand> curCommands;
	SizeType queueLastIndex = 0;

	IFrameBuffer* renderTarget = camera->renderTarget ? camera->renderTarget : scene->frameBuffer;

	curCamera = camera;

	int viewWidth, viewHeight;
	renderTarget->GetSize(viewWidth, viewHeight);
	renderTarget->Clear();

	float sp = scene->ScreenPercentage() / 100.f;

	if (scene->depth)
		scene->depth->Clear();

	scene->colorBuffer->Clear();

	if (viewWidth > scene->GetFrameBufferWidth() || viewHeight > scene->GetFrameBufferHeight())
		scene->ResizeBuffers(viewWidth, viewHeight);

	gGHI->SetViewport(0.f, 0.f, (float)viewWidth * sp, (float)viewHeight * sp);

	camera->CalculateMatrix((float)viewWidth / (float)viewHeight);

	FMatrix camMatrix = camera->projection * camera->view;
	float exposure = 1.0f;
	float gamma = 1.6f;

	CPostProcessVolumeProxy* ppVolumeA = nullptr;
	CPostProcessVolumeProxy* ppVolumeB = nullptr; // for if we have a volume with fading.

	// figure out which volume has the highest priority
	for (auto& volume : scene->GetPostProcessVolumes())
	{
		if (volume->GetMaterial() || !volume->IsEnabled())
			continue;

		if (volume->IsGlobal())
		{
			if (!ppVolumeA)
				ppVolumeA = volume;
			else if (volume->GetPriority() > ppVolumeA->GetPriority())
				ppVolumeA = volume;
			continue;
		}

		FVector relCamPos = volume->Rotation().Rotate(camera->position - volume->Bounds().position) + volume->Bounds().position;
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
		camera->position, 0u, camera->GetForwardVector(), 0u, scene->GetTime(), exposure, gamma, 0,
		FVector2((float)viewWidth, (float)viewHeight) / FVector2((float)scene->GetFrameBufferWidth(), (float)scene->GetFrameBufferHeight()),
		FVector2(viewWidth, viewHeight)
	};
	sceneBuffer->Update(sizeof(FSceneInfoBuffer), &sceneInfo);

	static TArray<TPair<CPrimitiveProxy*, FMeshBuilder>> dynamicMeshes;
	dynamicMeshes.Clear();

	for (auto* primitive : scene->GetPrimitives())
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
		GetRenderCommands(R_DEFERRED_PASS, scene->GetRenderQueue(), curCommands);

		curMeshes.Clear();
		GetMeshesToDraw(R_DEFERRED_PASS, dynamicMeshes, curMeshes);

		gGHI->SetBlendMode(EBlendMode::BLEND_DISABLED);

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

		gGHI->SetViewport(0.f, 0.f, (float)viewWidth * sp, (float)viewHeight * sp);
		gGHI->SetFrameBuffers(fbList, 5, scene->depth);

		gGHI->SetShaderBuffer(sceneBuffer, 1);
		gGHI->SetShaderBuffer(objectBuffer, 3);

		gGHI->SetShaderResource(scene->preTranslucentBuff, 3);

		IShader* curVsShader = nullptr;
		IShader* curPsShader = nullptr;

		for (auto& rc : curMeshes)
		{
			FObjectInfoBuffer objectInfo;
			objectInfo.transform = rc.Value->transform;
			objectInfo.position = rc.Key->GetPosition();
			memcpy(objectInfo.skeletonMatrices, rc.Value->skeletonMatrices.Data(), FMath::Min((int)rc.Value->skeletonMatrices.Size(), 48) * sizeof(FMatrix));
			objectBuffer->Update(sizeof(FObjectInfoBuffer), &objectInfo);

			IShader* _shader = rc.Value->mat->GetVsShader(ShaderType_DeferredPass);
			if (_shader != curVsShader)
			{
				curVsShader = _shader;
				gGHI->SetVsShader(curVsShader);
			}
			_shader = rc.Value->mat->GetPsShader(ShaderType_DeferredPass);
			if (_shader != curPsShader)
			{
				curPsShader = _shader;
				gGHI->SetPsShader(curPsShader);
			}

			gGHI->DrawMesh(rc.Value);
		}

		for (auto& rc : curCommands)
		{
			if (rc.type != FRenderCommand::DRAW_MESH)
				continue;

			FObjectInfoBuffer objectInfo{ { FMatrix(1.f) }, rc.drawMesh.transform, FVector(), 0 };
			objectBuffer->Update(sizeof(FObjectInfoBuffer), &objectInfo);

			gGHI->SetVsShader(rc.drawMesh.material->GetVsShader(ShaderType_DeferredPass));
			gGHI->SetPsShader(rc.drawMesh.material->GetPsShader(ShaderType_DeferredPass));

			gGHI->DrawMesh(&rc.drawMesh);
		}

		PreDeferredLightSetup(scene);

		gGHI->SetBlendMode(EBlendMode::BLEND_ADDITIVE_COLOR);

		gGHI->CopyResource(scene->depth, scene->depthTex);

		// -- Directional Lights --
		gGHI->SetPsShader(shaderDeferredDirLight->GetShader(ShaderType_Fragment));
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

				deferredLightBuffer->Update(sizeof(data), &data);
				gGHI->SetShaderResource(sunLightShadows, 2);

				FMesh mesh;
				mesh.numVertices = 3;
				gGHI->DrawMesh(&mesh);
			}
		}

		// -- Point Lights --
		gGHI->SetVsShader(shaderDeferredPointLight->GetShader(ShaderType_Vertex));
		gGHI->SetPsShader(shaderDeferredPointLight->GetShader(ShaderType_Fragment));
		gGHI->SetShaderBuffer(objectBuffer, 3);
		gGHI->SetFaceCulling(false);

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

				deferredLightBuffer->Update(sizeof(data), &data);

				FObjectInfoBuffer objectInfo;
				objectInfo.position = light->position;
				objectInfo.transform = FMatrix(1.f).Translate(data.position).Scale((light->range * 2) + 0.15f);
				objectBuffer->Update(sizeof(FObjectInfoBuffer), &objectInfo);

				const FMesh& mesh = meshIcoSphere->GetMeshes()[0];
				gGHI->DrawMesh((FMesh*)&mesh);
			}
		}

		UnlockGPU();
	}
#endif

	gGHI->SetFaceCulling(true);
	gGHI->SetBlendMode(EBlendMode::BLEND_DISABLED);

	{
		int matMode = cvRenderMaterialMode.AsInt();
		if (matMode == 1)
			Blit(scene->GBufferC, scene->colorBuffer);
		if (matMode == 2)
			Blit(scene->GBufferA, scene->colorBuffer);
	}

	//gRenderer->CopyResource(scene->colorBuffer, scene->preTranslucentBuff);
	gGHI->SetViewport(0.f, 0.f, (float)viewWidth * sp, (float)viewHeight * sp);

	// ------------- FORWARD PASS -------------
	{
		curCommands.Clear();
		GetRenderCommands(R_FORWARD_PASS, scene->GetRenderQueue(), curCommands);

		curMeshes.Clear();
		GetMeshesToDraw(R_FORWARD_PASS, dynamicMeshes, curMeshes);

		LockGPU();
		gGHI->SetBlendMode(EBlendMode::BLEND_DISABLED);

		gGHI->SetFrameBuffer(scene->colorBuffer, scene->depth);
		gGHI->SetShaderBuffer(sceneBuffer, 1);
		gGHI->SetShaderBuffer(forwardLightsBuffer, 2);
		gGHI->SetShaderBuffer(objectBuffer, 3);
		gGHI->SetShaderBuffer(shadowDataBuffer, 4);

		gGHI->SetShaderResource(sunLightShadows, 2);
		gGHI->SetShaderResource(scene->preTranslucentBuff, 3);

		IShader* overridePsShader = nullptr;
		if (cvRenderMaterialMode.AsInt() == 1 || scene->GetLights().Size() == 0)
			overridePsShader = debugUnlit->GetShader(ShaderType_Fragment);
		else if (cvRenderMaterialMode.AsInt() == 2)
			overridePsShader = debugNormalForward->GetShader(ShaderType_Fragment);

		if (overridePsShader)
			gGHI->SetPsShader(overridePsShader);

		for (auto& rc : curMeshes)
		{
			//FObjectInfoBuffer objectInfo{ *rc->skeletonMatrices.Data(), rc->transform, FVector(), 0 };
			FObjectInfoBuffer objectInfo;
			objectInfo.transform = rc.Value->transform;
			objectInfo.position = rc.Key->GetPosition();
			memcpy(objectInfo.skeletonMatrices, rc.Value->skeletonMatrices.Data(), FMath::Min((int)rc.Value->skeletonMatrices.Size(), 48) * sizeof(FMatrix));
			objectBuffer->Update(sizeof(FObjectInfoBuffer), &objectInfo);

			FForwardLightsBuffer lights{};
			CreateForwardLightBuffer(objectInfo.position, scene->GetLights(), lights);
			forwardLightsBuffer->Update(sizeof(FForwardLightsBuffer), &lights);

			gGHI->SetVsShader(rc.Value->mat->GetVsShader(ShaderType_ForwardPass));
			if (!overridePsShader)
				gGHI->SetPsShader(rc.Value->mat->GetPsShader(ShaderType_ForwardPass));

			gGHI->DrawMesh(rc.Value);
		}

		for (auto& rc : curCommands)
		{
			if (rc.type != FRenderCommand::DRAW_MESH)
				continue;

			FObjectInfoBuffer objectInfo{ { FMatrix(1.f) }, rc.drawMesh.transform, FVector(), 0 };
			objectBuffer->Update(sizeof(FObjectInfoBuffer), &objectInfo);

			gGHI->SetVsShader(rc.drawMesh.material->GetVsShader(ShaderType_ForwardPass));
			if (!overridePsShader)
				gGHI->SetPsShader(rc.drawMesh.material->GetPsShader(ShaderType_ForwardPass));

			gGHI->DrawMesh(&rc.drawMesh);
		}

		UnlockGPU();
	}

	gGHI->CopyResource(scene->colorBuffer, scene->preTranslucentBuff);
	gGHI->SetViewport(0.f, 0.f, (float)viewWidth * sp, (float)viewHeight * sp);

	// ------------- FORWARD TRANSPARENT PASS -------------
	{
		curCommands.Clear();
		GetRenderCommands(R_TRANSPARENT_PASS, scene->GetRenderQueue(), curCommands);

		curMeshes.Clear();
		GetMeshesToDraw(R_TRANSPARENT_PASS, dynamicMeshes, curMeshes);

		LockGPU();
		gGHI->SetBlendMode(EBlendMode::BLEND_ADDITIVE);

		gGHI->SetFrameBuffer(scene->colorBuffer, scene->depth);
		gGHI->SetShaderBuffer(sceneBuffer, 1);
		gGHI->SetShaderBuffer(forwardLightsBuffer, 2);
		gGHI->SetShaderBuffer(objectBuffer, 3);

		gGHI->SetShaderResource(sunLightShadows, 2);
		gGHI->SetShaderResource(scene->preTranslucentBuff, 3);

		IShader* overridePsShader = nullptr;
		if (cvRenderMaterialMode.AsInt() == 1 || scene->GetLights().Size() == 0)
			overridePsShader = debugUnlit->GetShader(ShaderType_ForwardPass);
		else if (cvRenderMaterialMode.AsInt() == 2)
			overridePsShader = debugNormalForward->GetShader(ShaderType_ForwardPass);

		if (overridePsShader)
			gGHI->SetPsShader(overridePsShader);

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
				memcpy(objectInfo.skeletonMatrices, rc.Value->skeletonMatrices.Data(), FMath::Min((int)rc.Value->skeletonMatrices.Size(), 48) * sizeof(FMatrix));
				objectBuffer->Update(sizeof(FObjectInfoBuffer), &objectInfo);

				FForwardLightsBuffer lights{};
				CreateForwardLightBuffer(objectInfo.position, scene->GetLights(), lights);
				forwardLightsBuffer->Update(sizeof(FForwardLightsBuffer), &lights);

				gGHI->SetVsShader(rc.Value->mat->GetVsShader(ShaderType_ForwardPass));
				if (!overridePsShader)
					gGHI->SetPsShader(rc.Value->mat->GetPsShader(ShaderType_ForwardPass));

				gGHI->DrawMesh(rc.Value);
			}
			else
			{
				FRenderCommand& rc = curCommands[i];
				if (rc.type != FRenderCommand::DRAW_MESH)
					continue;

				FObjectInfoBuffer objectInfo{ { FMatrix(1.f) }, rc.drawMesh.transform, FVector(), 0 };
				objectBuffer->Update(sizeof(FObjectInfoBuffer), &objectInfo);

				gGHI->SetVsShader(rc.drawMesh.material->GetVsShader(ShaderType_ForwardPass));
				if (!overridePsShader)
					gGHI->SetPsShader(rc.drawMesh.material->GetPsShader(ShaderType_ForwardPass));

				gGHI->DrawMesh(&rc.drawMesh);
			}
		}

		UnlockGPU();
	}

	gGHI->CopyResource(scene->colorBuffer, scene->preTranslucentBuff);

	// ------------- POST PROCESSING -------------
	{
		std::multimap<int, CPostProcessVolumeProxy*> postProcessMats;

		for (auto& volume : scene->GetPostProcessVolumes())
		{
			if (!volume->GetMaterial())
				continue;

			if (!volume->IsEnabled())
				continue;

			if (volume->IsGlobal() || volume->GetInfluence(camera) == 1.f)
				postProcessMats.insert(std::pair(volume->GetPriority(), volume));
		}

		LockGPU();
		{
			// TODO: dynamic exposure
			gGHI->SetBlendMode(EBlendMode::BLEND_DISABLED);
			gGHI->SetFrameBuffer(scene->colorBuffer, nullptr);

			gGHI->SetVsShader(shaderScreenPlane->GetShader(ShaderType_Vertex));
			gGHI->SetPsShader(shaderPPExposure->GetShader(ShaderType_Fragment));

			gGHI->SetShaderResource(scene->preTranslucentBuff, 0);

			FMesh mesh;
			mesh.numVertices = 3;
			gGHI->DrawMesh(&mesh);
		}

		gGHI->CopyResource(scene->colorBuffer, scene->preTranslucentBuff);

		if (cvRenderBloomEnabled.AsBool())
		{
			UnlockGPU();

			// Bloom Pass
			static TObjectPtr<IShaderBuffer> bloomInfoBuffer;
			if (!bloomInfoBuffer)
				bloomInfoBuffer = gGHI->CreateShaderBuffer(nullptr, sizeof(FBloomSettings));

			FBloomSettings bloomSettings{ cvRenderBloomIntensity.AsFloat(), cvRenderBloomThreshold.AsFloat(), cvRenderBloomKnee.AsFloat() };
			if (ppVolumeA)
			{
				bloomSettings.intensity = ppVolumeA->PostProcessSettings().bloomIntensity;
				bloomSettings.threshold = ppVolumeA->PostProcessSettings().bloomThreshold;
				bloomSettings.knee = ppVolumeA->PostProcessSettings().bloomKnee;
			}

			bloomInfoBuffer->Update(sizeof(FBloomSettings), &bloomSettings);

			gGHI->SetViewport(0, 0, viewWidth / 2, viewHeight / 2);
			gGHI->SetFrameBuffer(scene->bloomBuffersX[0]);
			gGHI->SetShaderResource(scene->colorBuffer, 0);
			gGHI->SetShaderBuffer(bloomInfoBuffer, 0);
			gGHI->SetVsShader(shaderScreenPlane->GetShader(ShaderType_Vertex));
			gGHI->SetPsShader(shaderBloomPreFilter->GetShader(ShaderType_Fragment));

			LockGPU();
			FMesh mesh;
			mesh.numVertices = 3;
			gGHI->DrawMesh(&mesh);

			static const int bloomScaleLUT[] = {
				2, 4, 8, 16, 32, 64
			};

			for (int i = 0; i < 4; i++)
			{
				gGHI->SetViewport(0, 0, viewWidth / bloomScaleLUT[i], viewHeight / bloomScaleLUT[i]);
				gGHI->SetVsShader(shaderScreenPlane->GetShader(ShaderType_Vertex));
				gGHI->SetPsShader(shaderGaussianBlurV->GetShader(ShaderType_Fragment));
				gGHI->SetFrameBuffer(scene->bloomBuffersY[i]);

				gGHI->SetShaderResource(scene->bloomBuffersX[i > 0 ? i - 1 : i], 0);

				gGHI->DrawMesh(&mesh);

				gGHI->SetFrameBuffer(scene->bloomBuffersX[i]);
				gGHI->SetShaderResource(scene->bloomBuffersY[i], 0);
				gGHI->SetPsShader(shaderGaussianBlurH->GetShader(ShaderType_Fragment));

				gGHI->DrawMesh(&mesh);

				//if (i + 1 < 4)
				//{
				//	UnlockGPU();
				//	// bloom
				//	Blit(scene->bloomBuffersX[i], scene->bloomBuffersX[i+1], FVector2(), FVector2(1, 1));
				//	LockGPU();
				//}
			}

			// Apply bloom
			gGHI->SetViewport(0.f, 0.f, (float)viewWidth * sp, (float)viewHeight * sp);
			gGHI->SetPsShader(shaderBloomPass->GetShader(ShaderType_Fragment));
			gGHI->SetFrameBuffer(scene->colorBuffer);
			gGHI->SetShaderResource(scene->preTranslucentBuff, 0);
			gGHI->SetShaderResource(scene->bloomBuffersX[0], 1);
			gGHI->SetShaderResource(scene->bloomBuffersX[1], 2);
			gGHI->SetShaderResource(scene->bloomBuffersX[2], 3);
			gGHI->SetShaderResource(scene->bloomBuffersX[3], 4);
			gGHI->SetShaderBuffer(bloomInfoBuffer, 0);

			gGHI->DrawMesh(&mesh);
		}

		for (auto& v : postProcessMats)
		{
			auto volume = v.second;

			gGHI->CopyResource(scene->colorBuffer, scene->preTranslucentBuff);

			gGHI->SetFrameBuffer(scene->colorBuffer, nullptr);
			gGHI->SetShaderResource(scene->preTranslucentBuff, 3);

			gGHI->SetVsShader(shaderScreenPlane->GetShader(ShaderType_Vertex));
			gGHI->SetPsShader(volume->GetMaterial()->GetPsShader(0));

			gGHI->SetMaterial(volume->GetMaterial());

			FMesh mesh;
			mesh.numVertices = 3;
			gGHI->DrawMesh(&mesh);
		}
		UnlockGPU();
	}

	// ------------- DEBUG PASS -------------
	for (int pass = 0; pass < 2; pass++)
	{
		curCommands.Clear();
		GetRenderCommands(pass == 0 ? R_DEBUG_PASS : R_DEBUG_OVERLAY_PASS, scene->GetRenderQueue(), curCommands);

		curMeshes.Clear();
		GetMeshesToDraw(pass == 0 ? R_DEBUG_PASS : R_DEBUG_OVERLAY_PASS, dynamicMeshes, curMeshes);

		LockGPU();
		gGHI->SetBlendMode(EBlendMode::BLEND_ADDITIVE);

		if (pass == 1)
			scene->depth->Clear();

		gGHI->SetViewport(0, 0, (float)viewWidth * sp, (float)viewHeight * sp);

		gGHI->SetFrameBuffer(scene->colorBuffer, scene->depth);
		gGHI->SetShaderBuffer(sceneBuffer, 1);
		gGHI->SetShaderBuffer(objectBuffer, 3);

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
				objectBuffer->Update(sizeof(FObjectInfoBuffer), &objectInfo);

				gGHI->SetVsShader(rc.drawMesh.material->GetVsShader(0));
				gGHI->SetPsShader(rc.drawMesh.material->GetPsShader(0));
				gGHI->DrawMesh(&rc.drawMesh);
			}

			if (type == 1)
			{
				TPair<CPrimitiveProxy*, FMeshBuilder::FRenderMesh*>& rc = curMeshes[i];

				FObjectInfoBuffer objectInfo;
				objectInfo.transform = rc.Value->transform;
				memcpy(objectInfo.skeletonMatrices, rc.Value->skeletonMatrices.Data(), FMath::Min((int)rc.Value->skeletonMatrices.Size(), 48));
				objectBuffer->Update(sizeof(FObjectInfoBuffer), &objectInfo);

				gGHI->SetVsShader(rc.Value->mat->GetVsShader(0));
				gGHI->SetPsShader(rc.Value->mat->GetPsShader(0));
				gGHI->DrawMesh(rc.Value);
			}
		}

		UnlockGPU();
	}
	Blit(scene->colorBuffer, renderTarget, FVector2(), FVector2((float)viewWidth / (float)scene->GetFrameBufferWidth(), (float)viewHeight / (float)scene->GetFrameBufferHeight()));
}

void CDefaultRenderer::RenderShadowMaps(CRenderScene* scene)
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

	GetRenderCommands(R_DEFERRED_PASS, scene->GetRenderQueue(), curCommands);
	GetRenderCommands(R_FORWARD_PASS, scene->GetRenderQueue(), curCommands);

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

	for (auto* primitive : scene->GetPrimitives())
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

	if (scene->GetPrimaryCamera() && sunLight)
	{
		sunLight->shadowIndex = 0;

		// Update sun light camera position
		FVector camPos = scene->GetPrimaryCamera()->position;
		camPos += scene->GetPrimaryCamera()->GetForwardVector() * 2;

		if (FVector::Distance(camPos, scene->sunLightCamPos) > 0.15f)
		{
			scene->sunLightCamPos = camPos;
			scene->sunLightCamDir = scene->GetPrimaryCamera()->GetForwardVector();
		}

		sunLightShadows->Clear();

		// Render directional light shadows
		gGHI->SetFrameBuffer(nullptr, sunLightShadows);

		for (int j = 0; j < FMath::Min(cvRenderShadowQuality.AsInt() + 1, 4); j++)
		{
			gGHI->SetViewport((float)(shadowTexSize * j), 0, (float)shadowTexSize, (float)shadowTexSize);

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
			sceneBuffer->Update(sizeof(FSceneInfoBuffer), &sceneInfo);

			gGHI->SetShaderBuffer(sceneBuffer, 1);
			gGHI->SetShaderBuffer(objectBuffer, 3);

			for (auto& rc : drawMeshes)
			{
				FObjectInfoBuffer objectInfo;
				objectInfo.transform = rc.Value.transform;
				objectInfo.position = rc.Key->GetPosition();
				memcpy(objectInfo.skeletonMatrices, rc.Value.skeletonMatrices.Data(), FMath::Min((int)rc.Value.skeletonMatrices.Size(), 48) * sizeof(FMatrix));
				objectBuffer->Update(sizeof(FObjectInfoBuffer), &objectInfo);

				gGHI->SetVsShader(rc.Value.mat->GetVsShader(0));
				gGHI->SetPsShader(nullptr);

				gGHI->DrawMesh(&rc.Value);
			}

			for (auto& rc : curCommands)
			{
				if (rc.type != FRenderCommand::DRAW_MESH)
					continue;

				FObjectInfoBuffer objectInfo{ { FMatrix(1.f) }, rc.drawMesh.transform, FVector(), 0 };
				objectBuffer->Update(sizeof(FObjectInfoBuffer), &objectInfo);

				gGHI->SetVsShader(rc.drawMesh.material->GetVsShader(0));
				gGHI->SetPsShader(nullptr);

				gGHI->DrawMesh(&rc.drawMesh);
			}
		}
	}

	shadowDataBuffer->Update(sizeof(FShadowDataBuffer), &shadowData);
}

void CDefaultRenderer::RenderUserInterface(CRenderScene* scene)
{

}
