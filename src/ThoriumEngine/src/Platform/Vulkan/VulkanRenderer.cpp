
#include "Engine.h"
#include "Window.h"
#include "VulkanRenderer.h"
#include "VulkanSwapChain.h"

#include <Util/Assert.h>

void VulkanRenderer::Init()
{
	type = ERendererType::VULKAN;

	THORIUM_ASSERT(gEngine->GetWindow(), "Attempting to initialize renderer with no active window.");

	VkApplicationInfo info{};
	info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	info.pApplicationName = gEngine->GetWindow()->GetWindowTitle().c_str();
	info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	info.pEngineName = "No Engine";
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

	THORIUM_ASSERT(dc != 0, "Failed to find device with Vulkan support!");

	VkDeviceCreateInfo deviceInfo{};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	
	result = vkCreateDevice(phys[0], &deviceInfo, nullptr, &device);
	THORIUM_ASSERT(result == VK_SUCCESS, "Failed to create vulkan device");



}

VulkanRenderer::~VulkanRenderer()
{
	vkDestroyDevice(device, nullptr);
	vkDestroyInstance(instance, nullptr);
}

IShader *VulkanRenderer::GetVsShader(CShaderSource *)
{
    return nullptr;
}

IShader *VulkanRenderer::GetPsShader(CShaderSource *)
{
    return nullptr;
}

IVertexBuffer *VulkanRenderer::CreateVertexBuffer(const TArray<FVertex> &)
{
    return nullptr;
}

IIndexBuffer *VulkanRenderer::CreateIndexBuffer(const TArray<uint> &)
{
    return nullptr;
}

IShaderBuffer *VulkanRenderer::CreateShaderBuffer(void *, SizeType)
{
    return nullptr;
}

ISwapChain *VulkanRenderer::CreateSwapChain(IBaseWindow *window)
{
    return nullptr;
}

IDepthBuffer *VulkanRenderer::CreateDepthBuffer(FDepthBufferInfo)
{
    return nullptr;
}

IFrameBuffer *VulkanRenderer::CreateFrameBuffer(int w, int h, ETextureFormat)
{
    return nullptr;
}

ITexture2D *VulkanRenderer::CreateTexture2D(void *data, int width, int height, ETextureFormat format, ETextureFilter filter)
{
    return nullptr;
}

ITexture2D *VulkanRenderer::CreateTexture2D(void **data, int numMipMaps, int width, int height, ETextureFormat format, ETextureFilter filter)
{
    return nullptr;
}

void VulkanRenderer::DrawMesh(FMesh *)
{
}

void VulkanRenderer::DrawMesh(FDrawMeshCmd *)
{
}

void VulkanRenderer::DrawMesh(FMeshBuilder::FRenderMesh *)
{
}

void VulkanRenderer::SetVsShader(IShader *shader)
{
}

void VulkanRenderer::SetPsShader(IShader *shader)
{
}

void VulkanRenderer::SetShaderBuffer(IShaderBuffer *, int r)
{
}

void VulkanRenderer::SetShaderResource(ITexture2D *texture, int _register)
{
}

void VulkanRenderer::SetShaderResource(IFrameBuffer *fb, int _register)
{
}

void VulkanRenderer::SetShaderResource(IDepthBuffer *depthTex, int _register)
{
}

void VulkanRenderer::SetFrameBuffer(IFrameBuffer *framebuffer, IDepthBuffer *depth)
{
}

void VulkanRenderer::SetFrameBuffers(IFrameBuffer **framebuffers, SizeType count, IDepthBuffer *depth)
{
}

void VulkanRenderer::SetViewport(float x, float y, float width, float height)
{
}

void VulkanRenderer::SetBlendMode(EBlendMode mode)
{
}

void VulkanRenderer::Present()
{
}

void VulkanRenderer::InitImGui(IBaseWindow *wnd)
{
}

void VulkanRenderer::ImGuiShutdown()
{
}

void VulkanRenderer::ImGuiBeginFrame()
{
}

void VulkanRenderer::ImGuiRender()
{
}
