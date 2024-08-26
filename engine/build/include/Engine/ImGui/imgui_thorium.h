#pragma once

#include <Util/String.h>
#include "imgui.h"

struct FVector;
struct FVector2;
struct FQuaternion;

namespace ImGui
{
	IMGUI_API bool InputText(const char* label, FString* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);

	// TableTreeHeader function return true when the node is open, in which case you need to also call TreePop() when you are finished displaying the tree node contents.
	IMGUI_API bool TableTreeHeader(const char* label, ImGuiTreeNodeFlags flags = 0, bool bUseTableBg = false);

	IMGUI_API bool Splitter(const char* id, bool bSplitVertical, float thickness, float* size0, float* size1, float padding = 10.f);

	IMGUI_API bool ImageButtonClear(const char* id, ImTextureID texture, const ImVec2& size, ImGuiButtonFlags flags = 0, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& tint_col = ImVec4(1, 1, 1, 1));

	IMGUI_API bool ButtonClear(const char* text, const ImVec2 size = ImVec2(0,0), ImGuiButtonFlags flags = 0);

	IMGUI_API void Text(const FString& txt);

	IMGUI_API bool TypeSelector(const char* label, int* v, int numOptions, const char** names, ImVec2 size = ImVec2(0, 24), float rounding = 5.f);
}
