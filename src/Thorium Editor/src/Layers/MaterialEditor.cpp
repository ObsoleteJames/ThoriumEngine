
#include "EditorEngine.h"
#include "MaterialEditor.h"
#include "Resources/ResourceManager.h"
#include "Resources/ModelAsset.h"
#include "Resources/Material.h"
#include "Game/World.h"
#include "Game/Entities/ModelEntity.h"
#include "Game/Components/PointLightComponent.h"
#include "Game/Components/SkyboxComponent.h"
#include "Rendering/RenderScene.h"
#include "Rendering/RenderProxies.h"

#include "FileDialogs.h"

#include "Platform/Windows/DirectX/DirectXFrameBuffer.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImGui/ImGui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_thorium.h"
#include "EditorWidgets.h"

CMaterialEditor::CMaterialEditor()
{
	previewModel = CResourceManager::GetResource<CModelAsset>(L"models\\Sphere.thmdl");

	scene = CreateObject<CWorld>();
	scene->InitWorld(CWorld::InitializeInfo().CreateRenderScene(true).RegisterForRendering(true));

	auto skyEnt = scene->CreateEntity<CEntity>();
	skyEnt->AddComponent<CSkyboxComponent>("skybox");

	modelEnt = scene->CreateEntity<CModelEntity>();
	modelEnt->SetModel(previewModel);

	fbView = gRenderer->CreateFrameBuffer(1280, 720, THTX_FORMAT_RGBA8_UINT);
	dbView = gRenderer->CreateDepthBuffer({ 1280, 720, TH_DBF_D24_S8, 1, false });

	camera = new CCameraProxy();
	camera->position = { 0, 0, -2 };

	light1 = modelEnt->AddComponent<CPointLightComponent>("Light1");
	light1->AttachTo(modelEnt->RootComponent());
	light1->SetPosition({ -1, 1, -1 });

	light2 = modelEnt->AddComponent<CPointLightComponent>("Light2");
	light2->AttachTo(modelEnt->RootComponent());
	light2->SetPosition({ 1, 0, -1 });

	openMatId = "MatEditorOpenMat" + FString::ToString((SizeType)this);
	saveMatId = "MatEditorSaveMat" + FString::ToString((SizeType)this);
}

void CMaterialEditor::SetMaterial(CMaterial* m, bool bNew)
{
	mat = m;
	bSaved = !bNew;
}

void CMaterialEditor::OnUpdate(double dt)
{
	int w, h;
	fbView->GetSize(w, h);
	if (w != viewportWidth || h != viewportHeight)
	{
		fbView->Resize(viewportWidth, viewportHeight);
		dbView->Resize(viewportWidth, viewportHeight);
	}

	scene->Update(dt);
	scene->GetRenderScene()->SetFrameBuffer(fbView);
	scene->GetRenderScene()->SetDepthBuffer(dbView);

	scene->SetPrimaryCamera(camera);

	modelEnt->modelComp->SetMaterial(mat, 0);
}

void CMaterialEditor::OnUIRender()
{
	FString title = "Material Editor";
	if (mat && mat->File())
		title += " - " + ToFString(mat->File()->Name());
	else if (mat)
		title += " - New Material";

	if (mat && !bSaved)
		title += '*';

	title += "###materialEditor_" + FString::ToString((SizeType)this);

	bool bOpen = true;
	ImGui::SetNextWindowSize(ImVec2(900, 620), ImGuiCond_FirstUseEver);

	WString f;
	WString m;
	if (ThFileDialog::AcceptFile(openMatId, &f) && !f.IsEmpty())
	{
		CMaterial* m = CResourceManager::GetResource<CMaterial>(f);
		if (m)
			SetMaterial(m);
	}
	if (ThFileDialog::AcceptFile(saveMatId, &f, &m) && !f.IsEmpty())
	{
		CResourceManager::RegisterNewResource(mat, f, m);
		mat->Save();
		bSaved = true;
	}

	int openPopup = 0;
	if (ImGui::Begin(title.c_str(), &bOpen, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoSavedSettings))
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New", "Ctrl+N"))
					openPopup = 1;
				if (ImGui::MenuItem("Open", "Ctrl+O"))
					openPopup = 2;
				if (ImGui::MenuItem("Save", "Ctrl+S"))
					SaveMat();
				ImGui::MenuItem("Save As");

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
			{
				ImGui::MenuItem("Undo");
				ImGui::MenuItem("Redo");
				ImGui::MenuItem("Revert");

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_S) && ImGui::IsKeyDown(ImGuiKey_ModCtrl))
			SaveMat();

		if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_N) && ImGui::IsKeyDown(ImGuiKey_ModCtrl))
			OnSaveNewMaterial(1);

		ImVec2 size = ImGui::GetContentRegionAvail();
		sizeR = size.x - sizeL;

		ImGui::Splitter("##assetBorwserSplitter", false, 4.f, &sizeL, &sizeR);

		if (ImGui::BeginChild("mateditorProperties", ImVec2(sizeL, 0), false, ImGuiWindowFlags_AlwaysUseWindowPadding))
		{
			if (!mat)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 0.4));
				ImGui::Text("No material loaded");
				ImGui::PopStyleColor();
				goto mateditorPropEnd;
			}
			else
				DrawProperties();
		}
	mateditorPropEnd:
		ImGui::EndChild();

		ImGui::SameLine();

		auto wndSize = ImGui::GetContentRegionAvail();
		auto cursorPos = ImGui::GetCursorScreenPos();
		viewportX = cursorPos.x;
		viewportY = cursorPos.y;

		DirectXFrameBuffer* fb = (DirectXFrameBuffer*)fbView;
		ImGui::Image(fb->view, { wndSize.x, wndSize.y });

		viewportWidth = FMath::Max((int)wndSize.x, 32);
		viewportHeight = FMath::Max((int)wndSize.y, 32);
	}
	ImGui::End();

	if (openPopup == 1)
	{
		if (!bSaved && mat)
		{
			saveCallback = &CMaterialEditor::OnSaveNewMaterial;
			ImGui::OpenPopup("Continue without saving?##_MATEDITCLOSE");
		}
		else
			OnSaveNewMaterial(1);
	}
	else if (openPopup == 2)
	{
		if (!bSaved && mat)
		{
			saveCallback = &CMaterialEditor::OnSaveOpenMaterial;
			ImGui::OpenPopup("Continue without saving?##_MATEDITCLOSE");
		}
		else
			ThFileDialog::OpenFile(openMatId, (FAssetClass*)CMaterial::StaticClass());
	}

	if (!bOpen)
	{
		bWantsToClose = true;
		if (!bSaved && mat)
		{
			saveCallback = &CMaterialEditor::OnSaveExit;
			ImGui::OpenPopup("Continue without saving?##_MATEDITCLOSE");
		}
		else
			gEditorEngine()->PollRemoveLayer(this);
	}

	int savePopupResult = -1;

	if (ImGui::BeginPopupModal("Continue without saving?##_MATEDITCLOSE"))
	{
		ImGui::Text("are you sure you want to continue without saving?");

		if (ImGui::Button("Save"))
		{
			SaveMat();
			ImGui::CloseCurrentPopup();
			savePopupResult = 1;
		}

		ImGui::SameLine();

		if (ImGui::Button("Don't Save"))
		{
			//gEditorEngine()->PollRemoveLayer(this);
			ImGui::CloseCurrentPopup();
			savePopupResult = 2;
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel"))
		{
			//bWantsToClose = false;
			ImGui::CloseCurrentPopup();
			savePopupResult = 0;
		}

		ImGui::EndPopup();
	}

	if (savePopupResult != -1 && saveCallback)
	{
		(this->*saveCallback)(savePopupResult);
		saveCallback = nullptr;
	}
}

void CMaterialEditor::OnDetach()
{
	scene->Delete();
	delete fbView;
	delete dbView;
	delete camera;
}

void CMaterialEditor::SaveMat()
{
	if (!mat)
		return;

	if (!mat->File())
	{
		ThFileDialog::SaveFile(saveMatId, (FAssetClass*)CMaterial::StaticClass());
		return;
	}

	mat->Save();
	bSaved = true;
}

void CMaterialEditor::DrawProperties()
{
	const auto& shaders = CShaderSource::GetAllShaders();

	if (ImGui::BeginCombo("Shader", mat->GetShaderSource()->shaderName.c_str()))
	{
		for (auto& s : shaders)
		{
			bool bSelected = s == mat->GetShaderSource();
			if (ImGui::Selectable(s->shaderName.c_str(), bSelected))
				mat->SetShader(s->shaderName);
			if (bSelected)
				ImGui::SetItemDefaultFocus();
		}

		ImGui::EndCombo();
	}

	for (auto& t : mat->textures)
	{
		TObjectPtr<CTexture>* tex = &t.tex;
		if (!t.bIsCustom)
			if (ImGui::AssetPtrWidget(t.name.c_str(), &tex, 1))
				bSaved = false;
	}


}

void CMaterialEditor::OnSaveOpenMaterial(int result)
{
	if (result != 0)
		ThFileDialog::OpenFile(openMatId, (FAssetClass*)CMaterial::StaticClass());
}

void CMaterialEditor::OnSaveExit(int result)
{
	// Cancel
	if (result == 0)
		bWantsToClose = false;
	else
		gEditorEngine()->PollRemoveLayer(this);
}

void CMaterialEditor::OnSaveNewMaterial(int result)
{
	if (result != 0)
	{
		CMaterial* mat = CreateObject<CMaterial>();
		mat->SetShader("Simple");
		SetMaterial(mat, true);
	}
}
