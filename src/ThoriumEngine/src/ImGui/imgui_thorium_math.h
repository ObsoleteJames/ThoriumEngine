#pragma once

#include "Math/Vectors.h"
#include "imgui.h"

namespace ImGui
{
	IMGUI_API bool DragVector(const char* label, FVector* v, float speed = 1.f, FVector min = FVector::zero, FVector max = FVector::zero, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
	IMGUI_API bool DragVector(const char* label, FVector2* v, float speed = 1.f, FVector2 min = FVector2(), FVector2 max = FVector2(), const char* format = "%.3f", ImGuiSliderFlags flags = 0);
	
	// min/max in degrees
	IMGUI_API bool DragQuat(const char* label, FQuaternion* v, float speed = 1.f, FVector min = FVector::zero, FVector max = FVector::zero, const char* format = "%.3f°", ImGuiSliderFlags flags = 0);
}
