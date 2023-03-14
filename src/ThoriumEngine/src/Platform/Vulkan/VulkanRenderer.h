#pragma once

#include "EngineCore.h"
#include "Rendering/Renderer.h"

#include <vulkan/vulkan.h>

class CVulkanRenderer : public IRenderer
{
public:
	CVulkanRenderer();
	virtual ~CVulkanRenderer();

	virtual void CompileShader(const FString& file) {}
	virtual IShader* CreateShader(const FString& type) { return nullptr; }

private:
	VkInstance instance;
	VkDevice device;

};
