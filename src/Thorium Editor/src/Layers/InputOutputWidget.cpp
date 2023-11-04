
#include <string>
#include "InputOutputWidget.h"
#include "Game/Entity.h"
#include "EditorEngine.h"

#include "Math/Color.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"

void CInputOutputWidget::OnUIRender()
{
	auto& selectedEntities = gEditorEngine()->selectedEntities;

	

	if (ImGui::Begin("Input/Output##_editorIOWidget", &bEnabled))
	{
		if (ImGui::BeginTable("_entIOBindingsTable", 3))
		{
			

			ImGui::EndTable();
		}
	}
	ImGui::End();
}
