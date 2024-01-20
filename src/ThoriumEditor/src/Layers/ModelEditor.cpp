
#include "ModelEditor.h"
#include "Game/World.h"
#include "Game/Entities/ModelEntity.h"
#include "Game/Components/SkyboxComponent.h"
#include "Game/Components/PointLightComponent.h"
#include "Rendering/RenderScene.h"

#include "EditorEngine.h"

#include "Platform/Windows/DirectX/DirectXFrameBuffer.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImGui/ImGui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_thorium.h"
#include "EditorWidgets.h"

CModelEditor::CModelEditor()
{
	scene = CreateObject<CWorld>();
	scene->InitWorld(CWorld::InitializeInfo().CreateRenderScene(true).RegisterForRendering(true));

	auto skyEnt = scene->CreateEntity<CEntity>();
	skyEnt->AddComponent<CSkyboxComponent>("skybox");

	modelEnt = scene->CreateEntity<CModelEntity>();

	framebuffer = gRenderer->CreateFrameBuffer(1280, 720, TEXTURE_FORMAT_RGBA8_UNORM);
	scene->GetRenderScene()->SetFrameBuffer(framebuffer);

	camera = new CCameraProxy();
	camera->position = { 0, 0, -2 };

	//scene->SetPrimaryCamera(camera);

	light1 = modelEnt->AddComponent<CPointLightComponent>("Light1");
	light1->AttachTo(modelEnt->RootComponent());
	light1->SetPosition({ -1, 1, -1 });

	openMdlId = "MdlEditorOpenMdl" + FString::ToString((SizeType)this);
	saveMdlId = "MdlEditorSaveMdl" + FString::ToString((SizeType)this);
}

void CModelEditor::SetModel(CModelAsset* mdl)
{
	this->mdl = mdl;
	modelEnt->SetModel(mdl);
}

void CModelEditor::OnUpdate(double dt)
{
	int w, h;
	framebuffer->GetSize(w, h);
	if (w != viewportWidth || h != viewportHeight)
	{
		framebuffer->Resize(FMath::Max(viewportWidth, 16), FMath::Max(viewportHeight, 16));
	}

	scene->Update(dt);
	scene->GetRenderScene()->SetFrameBuffer(framebuffer);

	scene->SetPrimaryCamera(camera);
}

void CModelEditor::OnUIRender()
{
	FString title = "Model Editor";
	if (mdl && mdl->File())
		title += " - " + mdl->File()->Name();
	else if (mdl)
		title += " - New Model";

	if (mdl && !bSaved)
		title += '*';

	title += "###modelEditor_" + FString::ToString((SizeType)this);

	bool bOpen = true;
	ImGui::SetNextWindowSize(ImVec2(960, 720), ImGuiCond_FirstUseEver);

	if (ImGui::Begin(title.c_str(), &bOpen, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoSavedSettings))
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New", "Ctrl+N"));
				if (ImGui::MenuItem("Open", "Ctrl+O"));
				if (ImGui::MenuItem("Save", "Ctrl+S") && mdl && mdl->File())
				{
					mdl->Save();
				}
				if (ImGui::MenuItem("Save As", "Ctrl+Shift+S"));

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Recalculate Bounds") && mdl)
				{
					for (auto& m : mdl->GetMeshes())
						m.CalculateBounds();
					mdl->CalculateBounds();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImVec2 size = ImGui::GetContentRegionAvail();
		sizeR = size.x - sizeL;

		ImGui::Splitter("##mdleditSplitter", false, 4.f, &sizeL, &sizeR, 100);

		if (ImGui::BeginChild("mdleditProps", ImVec2(sizeL, 0), false, ImGuiWindowFlags_AlwaysUseWindowPadding))
		{

		}
		ImGui::EndChild();

		ImGui::SameLine();

		auto wndSize = ImGui::GetContentRegionAvail();
		auto cursorPos = ImGui::GetCursorScreenPos();
		viewportX = cursorPos.x;
		viewportY = cursorPos.y;

		DirectXFrameBuffer* fb = (DirectXFrameBuffer*)framebuffer;
		ImGui::Image(fb->view, { wndSize.x, wndSize.y });

		if (ImGui::BeginDragDropTarget())
		{
			if (auto* p = ImGui::AcceptDragDropPayload("THORIUM_ASSET_FILE"); p != nullptr)
			{
				FFile* file = *(FFile**)p->Data;
				FAssetClass* type = CResourceManager::GetResourceTypeByFile(file);
				if (type == (FAssetClass*)CModelAsset::StaticClass())
				{
					SetModel(CResourceManager::GetResource<CModelAsset>(file->Path()));
				}
			}
			ImGui::EndDragDropTarget();
		}

		viewportWidth = FMath::Max((int)wndSize.x, 32);
		viewportHeight = FMath::Max((int)wndSize.y, 32);
	}
	ImGui::End();

	if (!bOpen)
	{
		bExit = true;

		gEditorEngine()->PollRemoveLayer(this);
	}
}

void CModelEditor::OnDetach()
{
	scene->Delete();
	delete framebuffer;
	delete camera;
}
