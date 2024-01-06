#pragma once

#include "VulkanRenderer.h"

#include "Rendering/Renderer.h"

class VulkanSwapChain : public ISwapChain
{
public:
    VulkanSwapChain(IBaseWindow* wnd);
    virtual ~VulkanSwapChain();

public:
    VkSurfaceKHR surface = nullptr;

};
