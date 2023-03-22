
#include "Renderer.h"
#include "RenderScene.h"
#include "Resources/Material.h"
#include "Game/Components/CameraComponent.h"
#include "Console.h"
#include <map>

#include <algorithm>

IRenderer* gRenderer = nullptr;
std::mutex IRenderer::gpuMutex;

std::thread renderThread;

// 0 = None, 1 = Unlit, 2 = Normal, 3 = Material
static CConVar cvRenderMaterialMode("r.materialmode");

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
			out.numSpotLights++;
		}
	}
}

void IRenderer::Init()
{
	CResourceManager::LoadResources<CShaderSource>();

	debugUnlit = CShaderSource::GetShader("Unlit");
	debugNormalForward = CShaderSource::GetShader("DebugNormalForward");

	debugUnlit->LoadShaderObjects();
	debugNormalForward->LoadShaderObjects();

	sceneBuffer = gRenderer->CreateShaderBuffer(nullptr, sizeof(FSceneInfoBuffer));
	objectBuffer = gRenderer->CreateShaderBuffer(nullptr, sizeof(FObjectInfoBuffer));
	forwardLightsBuffer = gRenderer->CreateShaderBuffer(nullptr, sizeof(FForwardLightsBuffer));
}

void IRenderer::renderAll()
{
	TArray<CRenderScene*>& renderQueue = gRenderer->renderScenes;

	// Draw each scene.
	for (auto scene : renderQueue)
	{
		if (!scene->camera)
			continue;

		TArray<FRenderCommand> curCommands;
		SizeType queueLastIndex = 0;

		int viewWidth, viewHeight;
		scene->frameBuffer->GetSize(viewWidth, viewHeight);
		scene->frameBuffer->Clear();
		if (scene->depthBuffer)
			scene->depthBuffer->Clear();

		gRenderer->SetViewport(0.f, 0.f, (float)viewWidth, (float)viewHeight);

		scene->camera->CalculateMatrix((float)viewWidth / (float)viewHeight);

		FSceneInfoBuffer sceneInfo{ scene->camera->GetProjectionMatrix() * scene->camera->GetViewMatrix(), 
			scene->camera->GetViewMatrix(), scene->camera->GetProjectionMatrix(),
			scene->camera->GetWorldPosition(), 0u, scene->camera->GetForwardVector(), 0u, scene->GetTime() };
		gRenderer->sceneBuffer->Update(sizeof(FSceneInfoBuffer), &sceneInfo);

		TArray<TPair<CPrimitiveProxy*, FMeshBuilder>> dynamicMeshes;
		
		for (auto* primitive : scene->primitves)
		{
			if (!primitive->IsVisible())
				continue;

			dynamicMeshes.Add({ primitive, FMeshBuilder() });
			FMeshBuilder& dynamic = dynamicMeshes.last()->Value;

			primitive->GetDynamicMeshes(dynamic);
		}

#if 0
		// ------------- SHADOW PASS -------------
		{
			{
				auto end = __getRenderCommands(R_SHADOW_PASS, scene->renderQueue.At(queueLastIndex), scene->renderQueue.end(), curCommands);
				queueLastIndex = ((SizeType)end.ptr - (SizeType)scene->renderQueue.Data()) / sizeof(FRenderCommand);
			}

			ILightComponent* sunLight;

			// Calculate shadow-map priority
			std::vector<TPair<float, ILightComponent*>> shadowLights;
			for (auto* light : scene->lights)
			{
				if (!light->CastShadows())
					continue;

				if (light->IsSunlight())
				{
					sunLight = light;
					continue;
				}

				float distance = FVector::Distance(scene->GetCamera()->GetWorldPosition(), light->GetWorldPosition());
				shadowLights.push_back({ distance, light });
			}

			struct {
				bool operator()(const TPair<float, ILightComponent*>& a, const TPair<float, ILightComponent*>& b) {
					return a.Key < b.Key;
				}
			} FCustomSort;

			std::sort(shadowLights.begin(), shadowLights.end(), FCustomSort);

			constexpr int maxShadows = 16;
			for (int i = 0; i < FMath::Min(16, (int)shadowLights.size()); i++)
			{
				// TODO: render shadows.
			}
		}

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

		// ------------- FORWARD PASS -------------
		{
			curCommands.Clear();
			GetRenderCommands(R_FORWARD_PASS, scene->renderQueue, curCommands);

			TArray<TPair<CPrimitiveProxy*, FMeshBuilder::FRenderMesh*>> forwardMeshes;
			GetMeshesToDraw(R_FORWARD_PASS, dynamicMeshes, forwardMeshes);

			LockGPU();
			gRenderer->SetFrameBuffer(scene->frameBuffer, scene->depthBuffer);
			gRenderer->SetShaderBuffer(gRenderer->sceneBuffer, 1);
			gRenderer->SetShaderBuffer(gRenderer->forwardLightsBuffer, 2);
			gRenderer->SetShaderBuffer(gRenderer->objectBuffer, 3);

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
				memcpy(objectInfo.skeletonMatrices, rc.Value->skeletonMatrices.Data(), FMath::Min(rc.Value->skeletonMatrices.Size(), 48ull));
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

		// ------------- DEBUG PASS -------------
		{
			curCommands.Clear();
			GetRenderCommands(R_DEBUG_PASS, scene->renderQueue, curCommands);

			TArray<TPair<CPrimitiveProxy*, FMeshBuilder::FRenderMesh*>> debugMeshes;
			GetMeshesToDraw(R_DEBUG_PASS, dynamicMeshes, debugMeshes);

			LockGPU();
			gRenderer->SetFrameBuffer(scene->frameBuffer, scene->depthBuffer);
			gRenderer->SetShaderBuffer(gRenderer->sceneBuffer, 1);
			gRenderer->SetShaderBuffer(gRenderer->objectBuffer, 3);

			for (auto& rc : curCommands)
			{
				if (rc.type != FRenderCommand::DRAW_MESH)
					continue;

				FObjectInfoBuffer objectInfo{ { FMatrix(1.f) }, rc.drawMesh.transform, FVector(), 0 };
				gRenderer->objectBuffer->Update(sizeof(FObjectInfoBuffer), &objectInfo);

				gRenderer->SetVsShader(rc.drawMesh.material->GetVsShader());
				gRenderer->SetPsShader(rc.drawMesh.material->GetPsShader());
				gRenderer->DrawMesh(&rc.drawMesh);
			}

			for (auto& rc : debugMeshes)
			{
				FObjectInfoBuffer objectInfo;
				objectInfo.transform = rc.Value->transform;
				memcpy(objectInfo.skeletonMatrices, rc.Value->skeletonMatrices.Data(), FMath::Min(rc.Value->skeletonMatrices.Size(), 48ull));
				gRenderer->objectBuffer->Update(sizeof(FObjectInfoBuffer), &objectInfo);

				gRenderer->SetVsShader(rc.Value->mat->GetVsShader());
				gRenderer->SetPsShader(rc.Value->mat->GetPsShader());
				gRenderer->DrawMesh(rc.Value);
			}

			UnlockGPU();
		}

		dynamicMeshes.Clear();
		scene->renderQueue.Clear();
	}

	gRenderer->renderScenes.Clear();
}
