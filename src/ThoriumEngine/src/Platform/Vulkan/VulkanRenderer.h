#pragma once

#include "EngineCore.h"
#include "Rendering/Renderer.h"

#include <vulkan/vulkan.h>

class VulkanRenderer : public IRenderer
{
public:
	VulkanRenderer();
	virtual ~VulkanRenderer();

	virtual void CompileShader(const FString& file) {}
	virtual IShader* CreateShader(const FString& type) { return nullptr; }

private:
	VkInstance instance;
	VkDevice device;

};
