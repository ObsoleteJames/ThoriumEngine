
#include "Engine.h"
#include "Window.h"
#include "Platform/Windows/DirectX/DirectXInterface.h"
#include "Platform/Windows/DirectX/DirectXFrameBuffer.h"
#include "Platform/Windows/DirectX/DirectXShader.h"
#include "Platform/Windows/DirectX/DirectXBuffers.h"
#include "Platform/Windows/DirectX/DirectXTexture.h"
#include "Console.h"
#include "Assets/Material.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3DCompiler.lib")
#include "D3dcompiler.h"
#include "d3d10sdklayers.h"

#include <Util/Assert.h>

#include <comdef.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_dx11.h"

DirectXInterface* GetDirectXRenderer()
{
	return (DirectXInterface*)gGHI;
}

void DirectXInterface::Init()
{
	api = EGraphicsApi::DIRECTX_11;

	CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factoryA);

	IDXGIAdapter* adapter;
	IDXGIAdapter* bestAdapter = nullptr;
	size_t bestAdapterMem = 0;
	for (int i = 0; factoryA->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++)
	{
		DXGI_ADAPTER_DESC desc;
		if (SUCCEEDED(adapter->GetDesc(&desc)))
		{
			if (bestAdapterMem < desc.DedicatedVideoMemory)
			{
				bestAdapterMem = desc.DedicatedVideoMemory;
				bestAdapter = adapter;
			}
		}		
	}

	THORIUM_ASSERT(bestAdapter, "Failed to find Graphics Adapter!");

	HRESULT hr = D3D11CreateDevice(
		bestAdapter,				// Adapter
		D3D_DRIVER_TYPE_UNKNOWN,	// Driver Type
		NULL,
#if CONFIG_RELEASE
		0,
#else
		D3D11_CREATE_DEVICE_DEBUG,
#endif
		NULL,
		0,
		D3D11_SDK_VERSION,
		&device,
		NULL,
		&deviceContext);

	_com_error err(hr);
	THORIUM_ASSERT(SUCCEEDED(hr), FString("Failed to create DirectX device\n") + err.ErrorMessage());

	// Get the factory object
	{
		IDXGIDevice2* pDXGIDevice;
		hr = device->QueryInterface(__uuidof(IDXGIDevice2), (void **)&pDXGIDevice);

		IDXGIAdapter* pDXGIAdapter;
		hr = pDXGIDevice->GetAdapter(&pDXGIAdapter);

		hr = pDXGIAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&factory);

		THORIUM_ASSERT(SUCCEEDED(hr), "Failed to get DirectX factory");
	}

	//D3D11_VIEWPORT viewport{};
	//viewport.TopLeftX = 0;
	//viewport.TopLeftY = 0;
	//viewport.Width = (float)width;
	//viewport.Height = (float)height;
	//viewport.MinDepth = 0.f;
	//viewport.MaxDepth = 1.f;

	//deviceContext->RSSetViewports(1, &viewport);

	//sceneBuffer = new DirectXShaderBuffer(nullptr, sizeof(FSceneInfoBuffer));
	//objectBuffer = new DirectXShaderBuffer(nullptr, sizeof(FObjectInfoBuffer));

	// Depth off State.
	{
		D3D11_DEPTH_STENCIL_DESC depthOffDesc{};
		depthOffDesc.DepthEnable = false;
		depthOffDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthOffDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		depthOffDesc.StencilEnable = true;
		depthOffDesc.StencilReadMask = 0xFF;
		depthOffDesc.StencilWriteMask = 0xFF;

		// Stencil operations if pixel is front-facing
		depthOffDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthOffDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depthOffDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthOffDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// Stencil operations if pixel is back-facing
		depthOffDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthOffDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depthOffDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthOffDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		device->CreateDepthStencilState(&depthOffDesc, &depthStateOff);
	}

	// Depth off State.
	{
		D3D11_DEPTH_STENCIL_DESC depthOffDesc{};
		depthOffDesc.DepthEnable = true;
		depthOffDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		depthOffDesc.DepthFunc = D3D11_COMPARISON_LESS;
		depthOffDesc.StencilEnable = true;
		depthOffDesc.StencilReadMask = 0xFF;
		depthOffDesc.StencilWriteMask = 0xFF;

		// Stencil operations if pixel is front-facing
		depthOffDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthOffDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depthOffDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthOffDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// Stencil operations if pixel is back-facing
		depthOffDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthOffDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depthOffDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthOffDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		device->CreateDepthStencilState(&depthOffDesc, &depthStateReadOnly);
	}

	// Blend State Additive
	{
		D3D11_BLEND_DESC blendDesc{};

		D3D11_RENDER_TARGET_BLEND_DESC rtbd{};

		rtbd.BlendEnable = true;
		rtbd.SrcBlend = D3D11_BLEND_SRC_ALPHA;
		rtbd.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		rtbd.BlendOp = D3D11_BLEND_OP_ADD;
		rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
		rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
		rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		blendDesc.RenderTarget[0] = rtbd;

		device->CreateBlendState(&blendDesc, &blendAdditive);
	}

	// Blend State Additive Color
	{
		D3D11_BLEND_DESC blendDesc{};

		D3D11_RENDER_TARGET_BLEND_DESC rtbd{};

		rtbd.BlendEnable = true;
		rtbd.SrcBlend = D3D11_BLEND_ONE;
		rtbd.DestBlend = D3D11_BLEND_ONE;
		rtbd.BlendOp = D3D11_BLEND_OP_ADD;
		rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
		rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
		rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		blendDesc.RenderTarget[0] = rtbd;

		device->CreateBlendState(&blendDesc, &blendAdditiveColor);
	}

	// Rasterizer state
	{
		D3D11_RASTERIZER_DESC desc{};
		desc.FillMode = D3D11_FILL_SOLID;
		desc.CullMode = D3D11_CULL_BACK;
		desc.FrontCounterClockwise = true;
		desc.DepthBias = false;
		desc.DepthBiasClamp = 0;
		desc.SlopeScaledDepthBias = 0;
		desc.DepthClipEnable = true;
		desc.ScissorEnable = false;
		desc.MultisampleEnable = false;
		desc.AntialiasedLineEnable = false;

		device->CreateRasterizerState(&desc, &rasterCullOff);
	}
}

void DirectXInterface::InitImGui(IBaseWindow* wnd)
{
	bImGuiGlfw = wnd->IsGlfwWindow();
	if (!wnd->IsGlfwWindow())
		ImGui_ImplWin32_Init(wnd->GetNativeHandle());
	else
		ImGui_ImplGlfw_InitForOther(((CWindow*)wnd)->GlfwWindow(), true);
	ImGui_ImplDX11_Init(device, deviceContext);
}

void DirectXInterface::ImGuiShutdown()
{
	ImGui_ImplDX11_Shutdown();
	if (!bImGuiGlfw)
		ImGui_ImplWin32_Shutdown();
	else
		ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

DirectXInterface::~DirectXInterface()
{
	factory->Release();
	deviceContext->Release();
	device->Release();
}

void DirectXInterface::CompileShader(const FString& source, IShader::EType shaderType, void** outBuffer, SizeType* outBufferSize)
{
	class CShaderInclude : public ID3DInclude
	{
	public:
		virtual HRESULT Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
		{
			FFile* file = CFileSystem::FindFile(FString("shaders/") + pFileName);
			if (!file)
				return S_FALSE;

			TUniquePtr<IBaseFStream> stream = file->GetStream("rb");
			if (!stream || !stream->IsOpen()) {
				return S_FALSE;
			}

			stream->Seek(0, SEEK_END);
			SizeType fileSize = stream->Tell();

			*ppData = malloc(fileSize);
			stream->Seek(0, SEEK_SET);

			stream->Read((void*)*ppData, fileSize);

			*pBytes = (uint)fileSize;

			return S_OK;
		}
		virtual HRESULT Close(LPCVOID pData) {
			free((void*)pData);
			return S_OK;
		}
	};

	ID3DBlob* outCode;
	ID3DBlob* errors;

	FString targetType;
	switch (shaderType)
	{
	case IShader::VFX_VS:
		targetType = "vs_5_0";
		break;
	case IShader::VFX_PS:
		targetType = "ps_5_0";
		break;
	case IShader::VFX_GEO:
		targetType = "gs_5_0";
		break;
	}

	*outBuffer = nullptr;

	CShaderInclude include;

#ifdef IS_DEV
	D3DCompile(source.c_str(), source.Size(), nullptr, nullptr, &include, "Main", targetType.c_str(), D3DCOMPILE_DEBUG, 0, &outCode, &errors);
#else
	D3DCompile(source.c_str(), source.Size(), nullptr, nullptr, &include, "Main", targetType.c_str(), 0, 0, &outCode, &errors);
#endif

	if (errors)
	{
		FString msg = (const char*)errors->GetBufferPointer();
		bool bHasErros = false;
		if (msg.Find("error") != -1)
			bHasErros = true;

		if (bHasErros)
		{
			CONSOLE_LogError("CRenderer", msg);
			errors->Release();
			return;
		}
		CONSOLE_LogWarning("CRenderer", msg);
		errors->Release();
	}

	*outBufferSize = outCode->GetBufferSize();
	*outBuffer = malloc(outCode->GetBufferSize());

	memcpy(*outBuffer, outCode->GetBufferPointer(), *outBufferSize);

	outCode->Release();
}

IShader* DirectXInterface::LoadShader(CShaderSource* source, EShaderType type, FString file)
{
	if (!source)
		return nullptr;

	if (type & ShaderType_Vertex)
		return new DirectXVertexShader(source, file);
	if (type & ShaderType_Fragment)
		return new DirectXShader(source, 1, file);
	if (type & ShaderType_Geometry)
		return new DirectXShader(source, 2, file);

	return nullptr;
}

IVertexBuffer* DirectXInterface::CreateVertexBuffer(const TArray<FVertex>& vertices)
{
	return new DirectXVertexBuffer(vertices);
}

IIndexBuffer* DirectXInterface::CreateIndexBuffer(const TArray<uint>& indices)
{
	return new DirectXIndexBuffer(indices);
}

IShaderBuffer* DirectXInterface::CreateShaderBuffer(void* data, SizeType size)
{
	return new DirectXShaderBuffer(data, size);
}

ISwapChain* DirectXInterface::CreateSwapChain(IBaseWindow* window)
{
	return new DirectXSwapChain(window);
}

IDepthBuffer* DirectXInterface::CreateDepthBuffer(FDepthBufferInfo depthInfo)
{
	return new DirectXDepthBuffer(depthInfo);
}

IFrameBuffer* DirectXInterface::CreateFrameBuffer(int width, int height, ETextureFormat format, ETextureFilter filter)
{
	return new DirectXFrameBuffer(width, height, format, filter);
}

IFrameBuffer* DirectXInterface::CreateFrameBuffer(int width, int height, int numMipMaps, ETextureFormat format, ETextureFilter filter /*= THTX_FILTER_LINEAR*/)
{
	return new DirectXFrameBuffer(width, height, numMipMaps, format, filter);
}

ITexture2D* DirectXInterface::CreateTexture2D(void* data, int width, int height, ETextureFormat format, ETextureFilter filter)
{
	auto* t = new DirectXTexture2D(data, width, height, format, filter);
	if (!t->tex || !t->view || !t->sampler)
	{
		delete t;
		return nullptr;
	}
	return t;
}

ITexture2D* DirectXInterface::CreateTexture2D(void** data, int numMipMaps, int width, int height, ETextureFormat format, ETextureFilter filter)
{
	auto* t = new DirectXTexture2D(data, numMipMaps, width, height, format, filter);
	if (!t->tex || !t->view || !t->sampler)
	{
		delete t;
		return nullptr;
	}
	return t;
}

ITextureCube* DirectXInterface::CreateTextureCube(void* data, int width, int height, ETextureFormat format, ETextureFilter filter)
{
	auto* t = new DirectXTextureCube(data, width, height, format, filter);
	if (!t->tex || !t->view || !t->sampler)
	{
		delete t;
		return nullptr;
	}
	return t;
}

void DirectXInterface::CopyResource(ITexture2D* source, ITexture2D* destination)
{
	DirectXTexture2D* s = (DirectXTexture2D*)source;
	DirectXTexture2D* d = (DirectXTexture2D*)destination;

	deviceContext->CopyResource(d->tex, s->tex);
}

void DirectXInterface::CopyResource(ITextureCube* source, ITextureCube* destination)
{
	DirectXTextureCube* s = (DirectXTextureCube*)source;
	DirectXTextureCube* d = (DirectXTextureCube*)destination;

	deviceContext->CopyResource(d->tex, s->tex);
}

void DirectXInterface::CopyResource(ITextureCube* source, ITexture2D* destination, int targetFace)
{
}

void DirectXInterface::CopyResource(IFrameBuffer* source, ITexture2D* destination)
{
	DirectXFrameBuffer* s = (DirectXFrameBuffer*)source;
	DirectXTexture2D* d = (DirectXTexture2D*)destination;

	deviceContext->CopyResource(d->tex, s->buffer);
}

void DirectXInterface::CopyResource(IFrameBuffer* source, IFrameBuffer* destination)
{
	DirectXFrameBuffer* s = (DirectXFrameBuffer*)source;
	DirectXFrameBuffer* d = (DirectXFrameBuffer*)destination;

	deviceContext->CopyResource(d->buffer, s->buffer);
}

void DirectXInterface::CopyResource(IDepthBuffer* source, ITexture2D* destination)
{
	DirectXDepthBuffer* s = (DirectXDepthBuffer*)source;
	DirectXTexture2D* d = (DirectXTexture2D*)destination;

	deviceContext->CopyResource(d->tex, s->depthBuffer);
}

void DirectXInterface::CopyResource(IDepthBuffer* source, IFrameBuffer* destination)
{
	DirectXDepthBuffer* s = (DirectXDepthBuffer*)source;
	DirectXFrameBuffer* d = (DirectXFrameBuffer*)destination;

	deviceContext->CopyResource(d->buffer, s->depthBuffer);
}

//void DirectXRenderer::BindGlobalData()
//{
//	deviceContext->VSSetConstantBuffers(1, 1, &((DirectXShaderBuffer*)&*sceneBuffer)->buffer);
//	deviceContext->VSSetConstantBuffers(3, 1, &((DirectXShaderBuffer*)&*objectBuffer)->buffer);
//
//	deviceContext->PSSetConstantBuffers(1, 1, &((DirectXShaderBuffer*)&*sceneBuffer)->buffer);
//	deviceContext->PSSetConstantBuffers(3, 1, &((DirectXShaderBuffer*)&*objectBuffer)->buffer);
//}

void DirectXInterface::DrawMesh(FMesh* mesh)
{
	uint vertexData[2] = { sizeof(FVertex), 0 };
	if (mesh->vertexBuffer)
		deviceContext->IASetVertexBuffers(0, 1, &((DirectXVertexBuffer*)&*mesh->vertexBuffer)->buffer, &vertexData[0], &vertexData[1]);
	if (mesh->indexBuffer)
		deviceContext->IASetIndexBuffer(((DirectXIndexBuffer*)&*mesh->indexBuffer)->buffer, DXGI_FORMAT_R32_UINT, 0);

	deviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->OMSetDepthStencilState(nullptr, 0);
	
	if (mesh->indexBuffer)
	{
		deviceContext->DrawIndexed(mesh->numIndices, 0, 0);
		gRenderStats.numTris += mesh->numIndices / 3;
	}
	else
		deviceContext->Draw(mesh->numVertices, 0);

	gRenderStats.numDrawCalls++;
}

void DirectXInterface::DrawMesh(FDrawMeshCmd* info)
{
	FMesh* mesh = info->mesh;
	info->material->UpdateGpuBuffer();

	deviceContext->IASetInputLayout(((DirectXVertexShader*)info->material->GetVsShader(0))->inputLayout);

	uint vertexData[2] = { sizeof(FVertex), 0 };
	if (mesh->vertexBuffer)
		deviceContext->IASetVertexBuffers(0, 1, &((DirectXVertexBuffer*)&*mesh->vertexBuffer)->buffer, &vertexData[0], &vertexData[1]);
	if (mesh->indexBuffer)
		deviceContext->IASetIndexBuffer(((DirectXIndexBuffer*)&*mesh->indexBuffer)->buffer, DXGI_FORMAT_R32_UINT, 0);

	D3D10_PRIMITIVE_TOPOLOGY topologyType = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	if (info->drawType & MESH_DRAW_PRIMITIVE_LINES)
		topologyType = D3D10_PRIMITIVE_TOPOLOGY_LINELIST;
	else if (info->drawType & MESH_DRAW_PRIMITIVE_POINTS)
		topologyType = D3D10_PRIMITIVE_TOPOLOGY_POINTLIST;

	deviceContext->IASetPrimitiveTopology(topologyType);

	DirectXShaderBuffer* matBuff = (DirectXShaderBuffer*)info->material->GetGpuBuffer();
	deviceContext->VSSetConstantBuffers(6, 1, &matBuff->buffer);
	deviceContext->PSSetConstantBuffers(6, 1, &matBuff->buffer);
	deviceContext->GSSetConstantBuffers(6, 1, &matBuff->buffer);

	for (auto& t : info->material->GetTextures())
	{
		if (!t.tex)
			continue;

		DirectXTexture2D* tex = (DirectXTexture2D*)t.tex->GetTextureObject();
		if (tex)
		{
			deviceContext->PSSetShaderResources(t.registerId, 1, &tex->view);
			deviceContext->PSSetSamplers(t.registerId, 1, &tex->sampler);
		}
	}

	//if (!info->material->DoDepthTest())
	//	deviceContext->OMSetDepthStencilState(depthStateOff, 0);
	//else
	//	deviceContext->OMSetDepthStencilState(nullptr, 0);

	//if (info->drawType & MESH_DRAW_DEPTH_WRITE != 0)
	//{
	//	if (info->drawType & MESH_DRAW_DEPTH_READ != 0)
	//		deviceContext->OMSetDepthStencilState(depthStateOff, 0);
	//	else
	//		deviceContext->OMSetDepthStencilState(depthStateReadOnly, 0);
	//}
	//else
	//	deviceContext->OMSetDepthStencilState(nullptr, 0);

	if (mesh->indexBuffer)
	{
		deviceContext->DrawIndexed(mesh->numIndices, 0, 0);
		gRenderStats.numTris += mesh->numIndices / 3;
	}
	else
		deviceContext->Draw(mesh->numVertices, 0);

	gRenderStats.numDrawCalls++;
}

void DirectXInterface::DrawMesh(FMeshBuilder::FRenderMesh* data)
{
	FMesh* mesh = &data->mesh;
	data->mat->UpdateGpuBuffer();

	deviceContext->IASetInputLayout(((DirectXVertexShader*)data->mat->GetVsShader(0))->inputLayout);

	uint vertexData[2] = { sizeof(FVertex), 0 };
	if (mesh->vertexBuffer)
		deviceContext->IASetVertexBuffers(0, 1, &((DirectXVertexBuffer*)&*mesh->vertexBuffer)->buffer, &vertexData[0], &vertexData[1]);
	if (mesh->indexBuffer)
		deviceContext->IASetIndexBuffer(((DirectXIndexBuffer*)&*mesh->indexBuffer)->buffer, DXGI_FORMAT_R32_UINT, 0);

	D3D10_PRIMITIVE_TOPOLOGY topologyType = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	if (mesh->topologyType == FMesh::TOPOLOGY_LINES)
		topologyType = D3D10_PRIMITIVE_TOPOLOGY_LINELIST;
	else if (mesh->topologyType == FMesh::TOPOLOGY_POINTS)
		topologyType = D3D10_PRIMITIVE_TOPOLOGY_POINTLIST;

	deviceContext->IASetPrimitiveTopology(topologyType);

	DirectXShaderBuffer* matBuff = (DirectXShaderBuffer*)data->mat->GetGpuBuffer();
	deviceContext->VSSetConstantBuffers(6, 1, &matBuff->buffer);
	deviceContext->PSSetConstantBuffers(6, 1, &matBuff->buffer);
	deviceContext->GSSetConstantBuffers(6, 1, &matBuff->buffer);

	for (auto& t : data->mat->GetTextures())
	{
		if (!t.tex)
			continue;

		DirectXTexture2D* tex = (DirectXTexture2D*)t.tex->GetTextureObject();
		if (tex)
		{
			deviceContext->PSSetShaderResources(t.registerId, 1, &tex->view);
			deviceContext->PSSetSamplers(t.registerId, 1, &tex->sampler);
		}
	}
	//if (!data->mat->DoDepthTest())
	//	deviceContext->OMSetDepthStencilState(depthStateOff, 0);
	//else
	//	deviceContext->OMSetDepthStencilState(nullptr, 0);

	if (topologyType == D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST && mesh->indexBuffer)
	{
		deviceContext->DrawIndexed(mesh->numIndices, 0, 0);
		gRenderStats.numTris += mesh->numIndices / 3;
	}
	else
		deviceContext->Draw(mesh->numVertices, 0);

	gRenderStats.numDrawCalls++;
}

void DirectXInterface::SetMaterial(CMaterial* mat)
{
	mat->UpdateGpuBuffer();

	DirectXShaderBuffer* matBuff = (DirectXShaderBuffer*)mat->GetGpuBuffer();
	deviceContext->VSSetConstantBuffers(6, 1, &matBuff->buffer);
	deviceContext->PSSetConstantBuffers(6, 1, &matBuff->buffer);
	deviceContext->GSSetConstantBuffers(6, 1, &matBuff->buffer);

	for (auto& t : mat->GetTextures())
	{
		if (!t.tex)
			continue;

		DirectXTexture2D* tex = (DirectXTexture2D*)t.tex->GetTextureObject();
		if (tex)
		{
			deviceContext->PSSetShaderResources(t.registerId, 1, &tex->view);
			deviceContext->PSSetSamplers(t.registerId, 1, &tex->sampler);
		}
	}
}

void DirectXInterface::SetVsShader(IShader* shader)
{
	deviceContext->VSSetShader(shader != nullptr ? (ID3D11VertexShader*)((DirectXShader*)shader)->shader : nullptr, nullptr, 0);
}

void DirectXInterface::SetPsShader(IShader* shader)
{
	deviceContext->PSSetShader(shader != nullptr ? (ID3D11PixelShader*)((DirectXShader*)shader)->shader : nullptr, nullptr, 0);
}

void DirectXInterface::SetShaderBuffer(IShaderBuffer* buffer, int _register)
{
	deviceContext->VSSetConstantBuffers(_register, 1, &((DirectXShaderBuffer*)&*buffer)->buffer);
	deviceContext->PSSetConstantBuffers(_register, 1, &((DirectXShaderBuffer*)&*buffer)->buffer);
	deviceContext->GSSetConstantBuffers(_register, 1, &((DirectXShaderBuffer*)&*buffer)->buffer);
}

void DirectXInterface::SetShaderResource(IBaseTexture* texture, int _register)
{
	if (texture->Type() == TextureType_2D)
	{
		DirectXTexture2D* tex = (DirectXTexture2D*)texture;
		deviceContext->PSSetShaderResources(_register, 1, &tex->view);
		deviceContext->PSSetSamplers(_register, 1, &tex->sampler);
	}
	else if (texture->Type() == TextureType_Cube)
	{
		DirectXTextureCube* tex = (DirectXTextureCube*)texture;
		deviceContext->PSSetShaderResources(_register, 1, &tex->view);
		deviceContext->PSSetSamplers(_register, 1, &tex->sampler);
	}
	else if (texture->Type() == TextureType_Framebuffer)
	{
		DirectXFrameBuffer* tex = (DirectXFrameBuffer*)texture;
		deviceContext->PSSetShaderResources(_register, 1, &tex->view);
		deviceContext->PSSetSamplers(_register, 1, &tex->sampler);
	}
}

void DirectXInterface::SetShaderResource(IDepthBuffer* depthTex, int _register)
{
	DirectXDepthBuffer* tex = (DirectXDepthBuffer*)depthTex;
	deviceContext->PSSetShaderResources(_register, 1, &tex->view);
	deviceContext->PSSetSamplers(_register, 1, &tex->sampler);
}

//void DirectXRenderer::SetShaderResource(IFrameBuffer* fb, int _register)
//{
//	DirectXFrameBuffer* tex = (DirectXFrameBuffer*)fb;
//	deviceContext->PSSetShaderResources(_register, 1, &tex->view);
//	deviceContext->PSSetSamplers(_register, 1, &tex->sampler);
//}

void DirectXInterface::SetFrameBuffer(IFrameBuffer* framebuffer, IDepthBuffer* depth)
{
	deviceContext->OMSetRenderTargets(framebuffer != nullptr, framebuffer != nullptr ? &((DirectXFrameBuffer*)framebuffer)->Get() : 0, depth != nullptr ? ((DirectXDepthBuffer*)depth)->depthView : 0);
}

void DirectXInterface::SetFrameBuffer(IFrameBuffer* framebuffer, int mip, IDepthBuffer* depth)
{
	deviceContext->OMSetRenderTargets(framebuffer != nullptr, framebuffer != nullptr ? &((DirectXFrameBuffer*)framebuffer)->Get(mip) : 0, depth != nullptr ? ((DirectXDepthBuffer*)depth)->depthView : 0);
}

void DirectXInterface::SetFrameBuffers(IFrameBuffer** framebuffers, SizeType count, IDepthBuffer* depth)
{
	TArray<ID3D11RenderTargetView*> arr(count);
	for (SizeType i = 0; i < count; i++)
		arr[i] = ((DirectXFrameBuffer*)framebuffers[i])->Get();

	deviceContext->OMSetRenderTargets((uint)count, arr.Data(), depth != nullptr ? ((DirectXDepthBuffer*)depth)->depthView : 0);
}

void DirectXInterface::SetViewport(float x, float y, float width, float height)
{
	D3D11_VIEWPORT viewport{};
	viewport.TopLeftX = x;
	viewport.TopLeftY = y;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;

	deviceContext->RSSetViewports(1, &viewport);
}

void DirectXInterface::SetBlendMode(EBlendMode mode)
{
	if (mode == EBlendMode::BLEND_DISABLED)
		deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
	else if (mode == EBlendMode::BLEND_ADDITIVE)
		deviceContext->OMSetBlendState(blendAdditive, nullptr, 0xFFFFFFFF);
	else if (mode == EBlendMode::BLEND_ADDITIVE_COLOR)
		deviceContext->OMSetBlendState(blendAdditiveColor, nullptr, 0xFFFFFFFF);
}

void DirectXInterface::SetFaceCulling(bool bEnabled)
{
	if (bEnabled)
		deviceContext->RSSetState(nullptr);
	else
		deviceContext->RSSetState(rasterCullOff);
}

void DirectXInterface::BindGBuffer()
{

}

void DirectXInterface::Present()
{
	//swapchain->Present(1, 0);
}

TPair<DXGI_FORMAT, int> DirectXInterface::GetDXTextureFormat(ETextureFormat format)
{
	static constexpr DXGI_FORMAT formats[] = {
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_R8_UNORM,
		DXGI_FORMAT_R8G8_UNORM,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_R10G10B10A2_UNORM,
		DXGI_FORMAT_R11G11B10_FLOAT,
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		DXGI_FORMAT_BC1_UNORM,
		DXGI_FORMAT_BC3_UNORM,
		DXGI_FORMAT_R24_UNORM_X8_TYPELESS
	};

	static constexpr int formatSizes[] = {
		0,
		1,
		2,
		3,
		4,
		4,
		4,
		8,
		16,
		2,
		4,
		4
	};

	return { formats[format], formatSizes[format] };
}

void DirectXInterface::ImGuiBeginFrame()
{
	ImGui_ImplDX11_NewFrame();
	if (!bImGuiGlfw)
		ImGui_ImplWin32_NewFrame();
	else
		ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void DirectXInterface::ImGuiRender()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

//void DirectXRenderer::Resize(int width, int height)
//{
//	CWindow* window = gEngine->GetWindow();
//	ID3D11Texture2D* backBuffer;
//
//	glfwGetFramebufferSize(window->GetNativeHandle(), &width, &height);
//
//	delete viewBuffer;
//	depthView->Release();
//	depthBuffer->Release();
//	swapchain->ResizeBuffers(0, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
//	swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
//	viewBuffer = new DirectXFrameBuffer(backBuffer);
//	backBuffer->Release();
//
//	CreateDepthBuffer();
//
//	D3D11_VIEWPORT viewport{};
//	viewport.TopLeftX = 0;
//	viewport.TopLeftY = 0;
//	viewport.Width = width;
//	viewport.Height = height;
//	viewport.MinDepth = 0.f;
//	viewport.MaxDepth = 1.f;
//
//	deviceContext->RSSetViewports(1, &viewport);
//}

//void DirectXRenderer::CreateDepthBufferOLD()
//{
//	D3D11_TEXTURE2D_DESC depthBuffInfo{};
//	depthBuffInfo.Width = width;
//	depthBuffInfo.Height = height;
//	depthBuffInfo.MipLevels = 1;
//	depthBuffInfo.ArraySize = 1;
//	depthBuffInfo.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
//	depthBuffInfo.SampleDesc.Count = 1;
//	depthBuffInfo.SampleDesc.Quality = 0;
//	depthBuffInfo.Usage = D3D11_USAGE_DEFAULT;
//	depthBuffInfo.BindFlags = D3D11_BIND_DEPTH_STENCIL;
//
//	HRESULT hr = device->CreateTexture2D(&depthBuffInfo, nullptr, &depthBuffer);
//	THORIUM_ASSERT(SUCCEEDED(hr), "Failed to create DirectX depth buffer");
//
//	hr = device->CreateDepthStencilView(depthBuffer, NULL, &depthView);
//	THORIUM_ASSERT(SUCCEEDED(hr), "Failed to create DirectX depth view");
//}
