#pragma once

#include "Rendering/Shader.h"

class COpenGLShader : public IShader
{
public:
	virtual ~COpenGLShader();

	void SetFloat(const FString& property, float value) override;
	void SetInt(const FString& property, int value) override;
	void SetBool(const FString& property, int8 value) override;
	bool PropertyExists(const FString& property) override;

	void UpdateShader(FShaderInfo* newSource) override;

	void Bind();
	void Unbind();

public:
	int program;

};
