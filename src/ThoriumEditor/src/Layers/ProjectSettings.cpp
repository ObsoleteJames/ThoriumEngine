
#include "ProjectSettings.h"
#include "EditorEngine.h"

#include "Game/GameInstance.h"
#include "Resources/ResourceManager.h"
#include "Resources/Scene.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_thorium.h"
#include "EditorWidgets.h"
#include "EditorMenu.h"

REGISTER_EDITOR_LAYER(CProjectSettingsWidget, "Edit/Project Settings", "Settings", false, false)

struct FPSMenu
{
	const char* name;
	void(CProjectSettingsWidget::*renderFunc)();
};

const FPSMenu menus[] = {
	{ "Game", &CProjectSettingsWidget::RenderGameSettings },
	{ "Audio", &CProjectSettingsWidget::RenderAudioSettings },
	{ "Input", &CProjectSettingsWidget::RenderInputSettings },
	{ "Physics", &CProjectSettingsWidget::RenderPhysicsSettings }
};

constexpr size_t menusCount = IM_ARRAYSIZE(menus);

void CProjectSettingsWidget::OnUIRender()
{
	if (ImGui::Begin("Project Settings", &bEnabled))
	{
		auto contentSize = ImGui::GetContentRegionAvail();

		if (ImGui::BeginChild("_psMenu", ImVec2(200, 0), false, ImGuiWindowFlags_AlwaysUseWindowPadding))
		{
			for (int i = 0; i < menusCount; i++)
			{
				if (ImGui::Selectable(menus[i].name, curMenu == i))
					curMenu = i;
			}
		}
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
		if (ImGui::BeginChild("_psSettings"))
		{
			ImGui::BeginDisabled(!gEngine->IsProjectLoaded());

			(this->*menus[curMenu].renderFunc)();

			ImGui::EndDisabled();
		}
		ImGui::EndChild();
		ImGui::PopStyleColor();

		/*if (ImGui::BeginTable("_psSettingsTable", 2))
		{


			ImGui::EndTable();
		}*/
	}
	ImGui::End();
	Menu()->bChecked = bEnabled;
}

void CProjectSettingsWidget::RenderGameSettings()
{
	ImGui::Text("Game Settings");
	
	ImVec2 region = ImGui::GetContentRegionAvail();
	ImVec2 cursor = ImGui::GetCursorScreenPos();

	ImGui::RenderFrame(cursor, cursor + region, ImGui::ColorConvertFloat4ToU32(ImVec4(0.1f, 0.1f, 0.1f, 1.f)), false);

	if (ImGui::BeginTable("_psSettingsTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("Project Name");
		ImGui::TableNextColumn();
		ImGui::InputText("##_projectNameEdit", (FString*)&gEditorEngine()->GetProjectConfig().displayName);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("Game Title");
		ImGui::TableNextColumn();
		ImGui::InputText("##_projectTitle", (FString*)&gEditorEngine()->ActiveGame().title);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("Icon");
		ImGui::TableNextColumn();
		TObjectPtr<CTexture> icon;
		if (!gEditorEngine()->GetProjectConfig().iconPath.IsEmpty())
		{
			icon = CResourceManager::GetResource<CTexture>(gEditorEngine()->GetProjectConfig().iconPath);
			if (!icon)
				gEditorEngine()->GetProjectConfig().iconPath.Clear();
		}
		TObjectPtr<CTexture>* iconPtr = &icon;
		if (ImGui::AssetPtrWidget("##_projectIcon", (TObjectPtr<CAsset>**) & iconPtr, 1, (FAssetClass*)CTexture::StaticClass()))
		{
			if (icon)
				gEditorEngine()->GetProjectConfig().iconPath = icon->GetPath();
			else
				gEditorEngine()->GetProjectConfig().iconPath.Clear();
		}

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("Author");
		ImGui::TableNextColumn();
		ImGui::InputText("##_projectAuthorEdit", (FString*)&gEditorEngine()->GetProjectConfig().author);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("Version");
		ImGui::TableNextColumn();
		ImGui::InputText("##_projectVersion", (FString*)&gEditorEngine()->ActiveGame().version, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsNoBlank);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("Game Instance Class");
		ImGui::TableNextColumn();
		TClassPtr<CObject>* gameInstanceEdit = (TClassPtr<CObject>*) & gEditorEngine()->ActiveGame().gameInstanceClass;
		ImGui::ClassPtrWidget("##_gameInstanceEdit", &gameInstanceEdit, 1, CGameInstance::StaticClass());
		
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("Startup Scene");
		ImGui::TableNextColumn();
		TObjectPtr<CScene> scene;
		if (!gEditorEngine()->ActiveGame().startupScene.IsEmpty())
		{
			scene = CResourceManager::GetResource<CScene>(ToFString(gEditorEngine()->ActiveGame().startupScene));
			if (!scene)
				gEditorEngine()->ActiveGame().startupScene = FString();
		}
		TObjectPtr<CScene>* scenePtr = &scene;
		if (ImGui::AssetPtrWidget("##_gameStartupScene", (TObjectPtr<CAsset>**) & scenePtr, 1, (FAssetClass*)CScene::StaticClass()))
		{
			if (scene)
				gEditorEngine()->ActiveGame().startupScene = ToFString(scene->File()->Path());
			else
				gEditorEngine()->ActiveGame().startupScene = FString();
		}

		ImGui::EndTable();
	}
}

void CProjectSettingsWidget::RenderAudioSettings()
{
	ImGui::Text("Audio Settings");
}

void CProjectSettingsWidget::RenderInputSettings()
{
	ImGui::Text("asd Settings");
}

void CProjectSettingsWidget::RenderPhysicsSettings()
{
	ImGui::Text("rwe Settings");
}
