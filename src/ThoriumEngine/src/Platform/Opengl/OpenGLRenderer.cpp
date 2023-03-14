
#include "OpenGLRenderer.h"
#include "OpenGLShader.h"

COpenGLRenderer::COpenGLRenderer()
{
	type = ERendererType::OPENGL;
}

COpenGLRenderer::~COpenGLRenderer()
{

}

void COpenGLRenderer::CompileShader(const FString& file)
{

}

IShader* COpenGLRenderer::CreateShader(const FString& type)
{
	return nullptr;
}
