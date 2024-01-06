
#define GLFW_INCLUDE_VULKAN
#include "glfw/glfw3.h"
#include "VulkanSwapChain.h"

#if _WIN32
#include <windows.h>
#endif

VulkanSwapChain::VulkanSwapChain(IBaseWindow *wnd)
{
#if _WIN32
    VkWin32SurfaceCreationInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = (HWND)window->GetNativeHandle();
    createInfo.hinstance = GetModuleHandle(nullptr);
#else
    glfwCreateWindowSurface

#endif


}

VulkanSwapChain::~VulkanSwapChain()
{
}
