#pragma once

#include "Rendering/Renderer.h"

class COpenGLRenderer : public IRenderer
{
public:
	COpenGLRenderer();
	virtual ~COpenGLRenderer();

	void CompileShader(const FString& file) override;
	IShader* CreateShader(const FString& type) override;

};
