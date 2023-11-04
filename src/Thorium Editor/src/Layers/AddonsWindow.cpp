
#include "AddonsWindow.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImGui/ImGui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_thorium.h"
#include "EditorWidgets.h"

CAddonsWindow::CAddonsWindow()
{

}

void CAddonsWindow::OnUpdate(double dt)
{

}

void CAddonsWindow::OnUIRender()
{
	if (ImGui::Begin("Addons", &bEnabled))
	{
		ImVec2 size = ImGui::GetContentRegionAvail();
		sizeR = size.x - sizeL - 5;

		ImGui::Splitter("##assetBorwserSplitter", false, 4.f, &sizeL, &sizeR, 32.f);

		if (ImGui::BeginChild("addonsLeft", ImVec2(sizeL, 0)))
		{

		}
		ImGui::EndChild();

		ImGui::SameLine();

		if (ImGui::BeginChild("addonsRight", ImVec2(sizeR, 0)))
		{

		}
		ImGui::EndChild();
	}
	ImGui::End();
}

void CAddonsWindow::DrawAddonList()
{

}

void CAddonsWindow::DrawAddonEditor()
{

}
