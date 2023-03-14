
#include "Engine.h"
#include "Window.h"
#include "VulkanRenderer.h"

#include <Util/Assert.h>

CVulkanRenderer::CVulkanRenderer()
{
	type = ERendererType::VULKAN;

	THORIUM_ASSERT(gEngine->GetWindow(), "Attempting to initialize renderer with no active window.");

	VkApplicationInfo info{};
	info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	info.pApplicationName = gEngine->GetWindow()->GetWindowTitle().c_str();
	info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	info.pEngineName = "ThoriumEngine";
	info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	info.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &info;

	uint32 glfwNumExtentions = 0;
	const char** glfwExtentions = glfwGetRequiredInstanceExtensions(&glfwNumExtentions);

	createInfo.enabledExtensionCount = glfwNumExtentions;
	createInfo.ppEnabledExtensionNames = glfwExtentions;
	createInfo.enabledLayerCount = 0;

	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
	THORIUM_ASSERT(result == VK_SUCCESS, "Failed to create vulkan instance");

	VkPhysicalDevice phys[4]; uint dc = 4;
	vkEnumeratePhysicalDevices(instance, &dc, phys);

	VkDeviceCreateInfo deviceInfo{};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	
	result = vkCreateDevice(phys[0], &deviceInfo, nullptr, &device);
	THORIUM_ASSERT(result == VK_SUCCESS, "Failed to create vulkan device");



}

CVulkanRenderer::~CVulkanRenderer()
{
	vkDestroyDevice(device, nullptr);
	vkDestroyInstance(instance, nullptr);
}
