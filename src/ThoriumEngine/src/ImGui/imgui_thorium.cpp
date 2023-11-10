
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_thorium.h"
#include "imgui_internal.h"

struct InputTextCallback_UserData
{
	FString* str;
	ImGuiInputTextCallback callback;
	void* callbackUserData;
};

static int InputTextCallback(ImGuiInputTextCallbackData* data)
{
	InputTextCallback_UserData* userData = (InputTextCallback_UserData*)data->UserData;
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
	{
		FString* str = userData->str;
		IM_ASSERT(data->Buf == str->c_str());
		str->Resize(data->BufTextLen);
		
		data->Buf = (char*)str->c_str();
	}
	else if (userData->callback)
	{
		data->UserData = userData->callbackUserData;
		return userData->callback(data);
	}
	return 0;
}

bool ImGui::InputText(const char* label, FString* str, ImGuiInputTextFlags flags /*= 0*/, ImGuiInputTextCallback callback /*= NULL*/, void* user_data /*= NULL*/)
{
	IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
	flags |= ImGuiInputTextFlags_CallbackResize;

	InputTextCallback_UserData data;
	data.str = str;
	data.callback = callback;
	data.callbackUserData = user_data;
	return InputText(label, (char*)str->c_str(), str->Capacity(), flags, InputTextCallback, &data);
}

IMGUI_API bool ImGui::TableTreeHeader(const char* label, ImGuiTreeNodeFlags flags /*= 0*/, bool bUseTableBg)
{
	flags |= ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FrameDontExpand;

	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 7));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0);
	ImGui::TableNextRow();
	ImGui::TableNextColumn();

	if (bUseTableBg)
		ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyle().Colors[ImGuiCol_TableRowBg]);

	bool bOpen = ImGui::TreeNodeEx(label, flags);
	ImGui::PopStyleVar(3);
	if (bUseTableBg)
		ImGui::PopStyleColor();

	return bOpen;
}

bool ImGui::Splitter(const char* _id, bool bVertical, float thickness, float* size0, float* size1, float padding /*= 10.f*/)
{
	ImVec2 backup_pos = ImGui::GetCursorPos();
	ImVec2 screenCursor = ImGui::GetCursorScreenPos();
	
	//ImVec2 regionSize = ImGui::GetContentRegionAvail();

	if (bVertical)
		ImGui::SetCursorPosY(backup_pos.y + *size0 + (thickness / 2));
	else
		ImGui::SetCursorPosX(backup_pos.x + *size0 + (thickness / 2));

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

	static float clickOffset = 0.f;

	bool b = ImGui::Button(_id, ImVec2(!bVertical ? thickness : -1.f, bVertical ? thickness : -1.f));
	if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
		clickOffset = bVertical ? (screenCursor.y + *size0) - ImGui::GetIO().MousePos.y : (screenCursor.x + *size0) - ImGui::GetIO().MousePos.x;

	ImGui::PopStyleColor(2);

	ImGui::SetItemAllowOverlap();

	if (ImGui::IsItemActive())
	{
		/*float mouseDelta = bVertical ? ImGui::GetIO().MouseDelta.y : ImGui::GetIO().MouseDelta.x;

		if (mouseDelta < padding - *size0)
			mouseDelta = padding - *size0;
		if (mouseDelta > *size1 - padding)
			mouseDelta = *size1 - padding;

		*size0 += mouseDelta;
		*size1 -= mouseDelta;*/

		float area = *size0 + *size1;
		float mousePos = (bVertical ? ImGui::GetIO().MousePos.y : ImGui::GetIO().MousePos.x) + clickOffset;
		float screenPos = bVertical ? screenCursor.y : screenCursor.x;
		mousePos -= screenPos;

		mousePos = mousePos < padding ? padding : mousePos;
		mousePos = mousePos > area - padding ? area - padding : mousePos;

		*size0 = mousePos;
		*size1 = area - mousePos;
	}
	ImGui::SetCursorPos(backup_pos);
	return b;

	//using namespace ImGui;
	//ImGuiContext& g = *ImGui::GetCurrentContext();
	//ImGuiWindow* window = g.CurrentWindow;
	//ImGuiID id = window->GetID(_id);
	//ImRect bb;
	//bb.Min = window->DC.CursorPos + (bVertical ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
	//bb.Max = bb.Min + CalcItemSize(bVertical ? ImVec2(thickness, -1) : ImVec2(-1, thickness), 0.0f, 0.0f);
	//return SplitterBehavior(bb, id, bVertical ? ImGuiAxis_X : ImGuiAxis_Y, size0, size1, padding, padding, 0.0f);
}

void ImGui::Text(const FString& txt)
{
	ImGui::Text(txt.c_str());
}
