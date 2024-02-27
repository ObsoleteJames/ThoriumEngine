
#include "EditorSettings.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_thorium.h"
#include "EditorWidgets.h"
#include "EditorEngine.h"
#include "EditorMenu.h"

REGISTER_EDITOR_LAYER(CEditorSettingsWidget, "Edit/Editor Settings", "Settings", false, false)

void CEditorSettingsWidget::OnUIRender()
{
	if (ImGui::Begin("Editor Settings", &bEnabled))
	{
		auto contentSize = ImGui::GetContentRegionAvail();

		if (ImGui::BeginChild("_esMenu", ImVec2(200, 0), false, ImGuiWindowFlags_AlwaysUseWindowPadding))
		{

		}
		ImGui::EndChild();
	}
	ImGui::End();
	Menu()->bChecked = bEnabled;
}
