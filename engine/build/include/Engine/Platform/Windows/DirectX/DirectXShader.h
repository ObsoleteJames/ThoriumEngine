#pragma once

#include "Rendering/Shader.h"
#include "DirectXRenderer.h"

class ENGINE_API DirectXShader : public IShader
{
public:
	DirectXShader(CShaderSource* shader, int type);
	virtual ~DirectXShader();

public:
	ID3D11DeviceChild* shader;

};

class ENGINE_API DirectXVertexShader : public IShader
{
public:
	DirectXVertexShader(CShaderSource* shader);
	virtual ~DirectXVertexShader();

public:
	ID3D11VertexShader* shader;
	ID3D11InputLayout* inputLayout;

};