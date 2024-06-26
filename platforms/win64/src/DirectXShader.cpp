
#include "Platform/Windows/DirectX/DirectXShader.h"
#include "Console.h"
#include <d3dcompiler.h>

DirectXShader::DirectXShader(CShaderSource* in, int type, FString path)
{
	bValid = false;
	shaderSource = in;
	this->type = (int8)type;

	//FString typeStr = (type == 0) ? ".vs.thcs" : (type == 1) ? ".thcs.ps" : ".thcs.gs";
	//FString shaderPath = in->File()->Mod()->Path() + "/" + in->File()->Dir()->GetPath() + "/vfx/" + in->File()->Name() + typeStr;
	CFStream stream(ToFString(path), "rb");
	if (!stream.IsOpen())
	{
		CONSOLE_LogError("IShader", FString("Failed to create shader for '") + in->shaderName + "'!");
		return;
	}

	stream.Seek(0, SEEK_END);
	SizeType shaderSize = stream.Tell();
	stream.Seek(0, SEEK_SET);

	void* buff = malloc(shaderSize);
	stream.Read(buff, shaderSize);

	switch (type)
	{
	case 0:
		free(buff);
		return;
	case 1:
		GetDirectXRenderer()->device->CreatePixelShader(buff, shaderSize, nullptr, (ID3D11PixelShader**)&shader);
		break;
	case 2:
		GetDirectXRenderer()->device->CreateGeometryShader(buff, shaderSize, nullptr, (ID3D11GeometryShader**)&shader);
		break;
	}

	free(buff);
	bValid = true;
}

DirectXShader::~DirectXShader()
{
	shader->Release();
}

DirectXVertexShader::DirectXVertexShader(CShaderSource* in, FString path)
{
	bValid = false;
	shaderSource = in;
	type = 0;

	//WString shaderPath = in->File()->Mod()->Path() + in->File()->Dir()->GetPath() + L"/vfx/" + in->File()->Name() + L".vs.thcs";
	//CFStream stream(ToFString(shaderPath), "rb");
	//if (!stream.IsOpen())
	//{
	//	CONSOLE_LogError(FString("Failed to create vertex shader for '") + in->shaderName + "'!");
	//	return;
	//}
	//
	//stream.Seek(0, SEEK_END);
	//SizeType shaderSize = stream.Tell();
	//stream.Seek(0, SEEK_SET);
	//
	//void* buff = malloc(shaderSize);
	//stream.Read(buff, shaderSize);

	ID3D10Blob* buff;

	//FString path = in->File()->Mod()->Path() + "/" + in->File()->Dir()->GetPath() + "/vfx/" + in->File()->Name() + ".thcs.vs";
	wchar_t p[MAX_PATH];
	mbstowcs(p, path.c_str(), path.Size() + 1);

	D3DReadFileToBlob(p, &buff);

	D3D11_INPUT_ELEMENT_DESC vertexLayout[] = {
		{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, offsetof(FVertex, position),			D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{ "NORMAL",			0, DXGI_FORMAT_R32G32B32_FLOAT,		0, offsetof(FVertex, normal),			D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{ "TANGENT",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, offsetof(FVertex, tangent),			D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{ "COLOR",			0, DXGI_FORMAT_R32G32B32_FLOAT,		0, offsetof(FVertex, color),			D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{ "TEXCOORD",		0, DXGI_FORMAT_R32G32_FLOAT,		0, offsetof(FVertex, uv1),				D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{ "TEXCOORD",		1, DXGI_FORMAT_R32G32_FLOAT,		0, offsetof(FVertex, uv2),				D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{ "BONEINDICES",	0, DXGI_FORMAT_R32G32B32A32_SINT,	0, offsetof(FVertex, bones),			D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{ "BONEWEIGHT",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, offsetof(FVertex, boneInfluence),	D3D11_INPUT_PER_VERTEX_DATA, 0  },
	};
	constexpr UINT vertexLayoutSize = ARRAYSIZE(vertexLayout);

	HRESULT hr = GetDirectXRenderer()->device->CreateInputLayout(vertexLayout, vertexLayoutSize, buff->GetBufferPointer(), buff->GetBufferSize(), &inputLayout);
	if (FAILED(hr))
		CONSOLE_LogError("IShader", "Failed to create input layout for vertex shader!");

	hr = GetDirectXRenderer()->device->CreateVertexShader(buff->GetBufferPointer(), buff->GetBufferSize(), nullptr, &shader);
	if (FAILED(hr))
	{
		CONSOLE_LogError("IShader", "Failed to create vertex shader!");
		buff->Release();
		return;
	}

	buff->Release();
	bValid = true;
}

DirectXVertexShader::~DirectXVertexShader()
{
	if (shader)
		shader->Release();
}
