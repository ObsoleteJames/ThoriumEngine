
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_thorium.h"
#include "imgui_internal.h"
#include "imgui_thorium_math.h"

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
		
		SizeType& _size = *(SizeType*)(((SizeType)str) + sizeof(void*) + sizeof(SizeType));
		_size = data->BufTextLen;
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

bool ImGui::ImageButtonClear(const char* str_id, ImTextureID texture, const ImVec2& size, ImGuiButtonFlags flags, const ImVec2& uv0 /*= ImVec2(0, 0)*/, const ImVec2& uv1 /*= ImVec2(1, 1)*/, const ImVec4& tint_col /*= ImVec4(1, 1, 1, 1)*/)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiID id = window->GetID(str_id);

	const ImVec2 padding = g.Style.FramePadding;
	const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size + padding * 2.0f);
	ItemSize(bb);
	if (!ItemAdd(bb, id))
		return false;

	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

	// Render
	auto col = (hovered && pressed) ? ImVec4(0.8f, 0.8f, 0.8f, 1.f) : (hovered ? ImVec4(0.9f, 0.9f, 0.9f, 1.f) : ImVec4(1, 1, 1, 1));
	RenderNavHighlight(bb, id);
	window->DrawList->AddImage(texture, bb.Min, bb.Max, uv0, uv1, GetColorU32(tint_col * col));

	return pressed;
}

bool ImGui::ButtonClear(const char* label, const ImVec2 size_arg /*= ImVec2(0,0)*/, ImGuiButtonFlags flags /*= 0*/)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
		pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
	ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ItemSize(size, style.FramePadding.y);
	if (!ItemAdd(bb, id))
		return false;

	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

	// Render
	//const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	RenderNavHighlight(bb, id);
	//RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);

	if (g.LogEnabled)
		LogSetNextTextDecoration("[", "]");

	auto col = (hovered && pressed) ? ImVec4(0.8f, 0.8f, 0.8f, 1.f) : (hovered ? ImVec4(0.9f, 0.9f, 0.9f, 1.f) : ImVec4(1, 1, 1, 1));
	ImGui::PushStyleColor(ImGuiCol_Text, col);
	RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);
	ImGui::PopStyleColor();

	IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
	return pressed;
}

void ImGui::Text(const FString& txt)
{
	ImGui::Text(txt.c_str());
}

bool ImGui::TypeSelector(const char* label, int* value, int numOptions, const char** names, ImVec2 size /*= 10.f*/, float rounding /*= 5.f*/)
{
	FString lbl = label;
	ImVec2 textSize = ImGui::CalcTextSize(label, 0, true);

	ImGuiStyle& style = ImGui::GetStyle();

	ImVec2 cursor = ImGui::GetCursorScreenPos();
	ImVec2 contentSize = size.x == 0.f ? ImGui::GetContentRegionAvail() : size;
	if (textSize.x > 0)
		contentSize.x -= textSize.x + style.FramePadding.x;

	float itemWidth = contentSize.x / numOptions;

	bool r = false;
	for (int i = 0; i < numOptions; i++)
	{
		ImVec2 itemPos = cursor + ImVec2(itemWidth * i, 0);
		ImVec2 itemSize = ImVec2(itemWidth, size.y);
		ImGui::SetCursorScreenPos(itemPos);
		if (ImGui::InvisibleButton(("##" + lbl + FString::ToString(i)).c_str(), itemSize))
		{
			r = true;
			*value = i;
		}

		bool bHovered = ImGui::IsItemHovered();
		bool bSelected = *value == i;

		// Draw button
		auto col = ImGui::ColorConvertFloat4ToU32(style.Colors[bHovered ? ImGuiCol_TabUnfocusedActive : (bSelected ? ImGuiCol_TabActive : ImGuiCol_Tab)]);
		ImGui::RenderFrame(itemPos, itemPos + itemSize, col, false, rounding);

		if (i < numOptions - 1)
		{
			ImGui::RenderFrame(itemPos + ImVec2(itemSize.x - rounding, 0), itemPos + itemSize, col, false);
			ImGui::RenderFrame(itemPos + ImVec2(itemSize.x - 1.f, 0), itemPos + itemSize, ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_FrameBg]), false);
		}
		if (i > 0)
		{
			ImGui::RenderFrame(itemPos, itemPos + ImVec2(rounding, itemSize.y), col, false);
		}

		ImVec2 nameSize = ImGui::CalcTextSize(names[i]);
		ImGui::RenderText(itemPos + (itemSize / 2) - (nameSize / 2), names[i]);
	}

	ImGui::RenderText(cursor + ImVec2(contentSize.x + style.FramePadding.x, 0), label);

	return r;
}

bool ImGui::DragVector(const char* label, FVector* v, float speed, FVector min, FVector max, const char* format, ImGuiSliderFlags flags)
{
	auto areaSize = ImGui::GetContentRegionAvail();
	auto lblSize = ImGui::CalcTextSize(label, nullptr, true);

	ImVec2 frameSize = ImVec2(2.f, (lblSize.y + ImGui::GetStyle().FramePadding.y * 2.0f) - 7.f);
	
	float tWidth = (areaSize.x - lblSize.x) / 3 - 5.f;

	FString lblX = FString("##x") + label;
	FString lblY = FString("##y") + label;
	FString lblZ = FString("##z") + label;

	bool r = false;

	ImVec2 cursorPos = ImGui::GetCursorScreenPos() + ImVec2(4, 4);

	ImGui::SetNextItemWidth(tWidth);
	if (ImGui::DragFloat(lblX.c_str(), &v->x, speed, min.x, max.x, format, flags))
		r = true;

	ImGui::RenderFrame(cursorPos, cursorPos + frameSize, ImGui::GetColorU32(ImVec4(0.7f, 0.2f, 0.2f, 1)), false, 1);

	ImGui::SameLine();
	cursorPos = ImGui::GetCursorScreenPos() + ImVec2(4, 4);

	ImGui::SetNextItemWidth(tWidth);
	if (ImGui::DragFloat(lblY.c_str(), &v->y, speed, min.y, max.y, format, flags))
		r = true;

	ImGui::RenderFrame(cursorPos, cursorPos + frameSize, ImGui::GetColorU32(ImVec4(0.2f, 0.7f, 0.2f, 1)), false, 1);

	ImGui::SameLine();
	cursorPos = ImGui::GetCursorScreenPos() + ImVec2(4, 4);

	ImGui::SetNextItemWidth(tWidth);
	if (ImGui::DragFloat(lblZ.c_str(), &v->z, speed, min.z, max.z, format, flags))
		r = true;

	ImGui::RenderFrame(cursorPos, cursorPos + frameSize, ImGui::GetColorU32(ImVec4(0.2f, 0.2f, 0.7f, 1)), false, 1);
	return r;
}

bool ImGui::DragVector(const char* label, FVector2* v, float speed, FVector2 min, FVector2 max, const char* format, ImGuiSliderFlags flags)
{
	auto areaSize = ImGui::GetContentRegionAvail();
	auto lblSize = ImGui::CalcTextSize(label, nullptr, true);

	ImVec2 frameSize = ImVec2(2.f, (lblSize.y + ImGui::GetStyle().FramePadding.y * 2.0f) - 7.f);

	float tWidth = (areaSize.x - lblSize.x) / 3 - 5.f;

	FString lblX = FString("##x") + label;
	FString lblY = FString("##y") + label;

	bool r = false;

	ImVec2 cursorPos = ImGui::GetCursorScreenPos() + ImVec2(4, 4);

	ImGui::SetNextItemWidth(tWidth);
	if (ImGui::DragFloat(lblX.c_str(), &v->x, speed, min.x, max.x, format, flags))
		r = true;

	ImGui::RenderFrame(cursorPos, cursorPos + frameSize, ImGui::GetColorU32(ImVec4(0.7f, 0.2f, 0.2f, 1)), false, 1);

	ImGui::SameLine();
	cursorPos = ImGui::GetCursorScreenPos() + ImVec2(4, 4);

	ImGui::SetNextItemWidth(tWidth);
	if (ImGui::DragFloat(lblY.c_str(), &v->y, speed, min.y, max.y, format, flags))
		r = true;

	ImGui::RenderFrame(cursorPos, cursorPos + frameSize, ImGui::GetColorU32(ImVec4(0.2f, 0.7f, 0.2f, 1)), false, 1);
	return r;
}

bool ImGui::DragQuat(const char* label, FQuaternion* v, float speed, FVector min, FVector max, const char* format, ImGuiSliderFlags flags)
{
	return false;
}
