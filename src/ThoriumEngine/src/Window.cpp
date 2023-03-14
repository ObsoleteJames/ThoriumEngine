
#define GLFW_EXPOSE_NATIVE_WIN32
#include "Window.h"
#include "Rendering/Renderer.h"
#include "Rendering/Framebuffer.h"
#include <Util/Assert.h>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

void CWindow::PollEvents()
{
	glfwPollEvents();
}

void CWindow::UpdateWindowRect()
{
	glfwGetWindowSize((GLFWwindow*)nativeHandle, &WindowedRect.w, &WindowedRect.h);
	glfwGetWindowPos((GLFWwindow*)nativeHandle, &WindowedRect.x, &WindowedRect.y);
}

TArray<CWindow*> CWindow::windows;

CWindow::CWindow(int w, int h, int x, int y, const FString& title, EWindowMode mode)
{
	windowTitle = title;
	width = w;
	height = h;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//glfwWindowHint(GLFW_RESIZABLE, false);

	nativeHandle = glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);
	THORIUM_ASSERT(nativeHandle, "Failed to create window");

	WindowedRect.x = x;
	WindowedRect.y = y;
	WindowedRect.w = w;
	WindowedRect.h = h;

	SetWindowMode(mode);
	windows.Add(this);

	glfwSetCursorPosCallback((GLFWwindow*)nativeHandle, [](GLFWwindow* wnd, double x, double y) {
		CWindow* w = GetWindowFromHandle(wnd);
		w->mouseX = x;
		w->mouseY = y;
		w->OnCursorMove.Fire(x, y);
	});
	glfwSetKeyCallback((GLFWwindow*)nativeHandle, [](GLFWwindow* wnd, int key, int scancode, int action, int mods) {
		CWindow* w = GetWindowFromHandle(wnd);
		w->OnKeyEvent.Fire((EKeyCode)key, (EInputAction)action, (EInputMod)mods);
	});
	glfwSetCharCallback((GLFWwindow*)nativeHandle, [](GLFWwindow* wnd, uint k) {
		CWindow* w = GetWindowFromHandle(wnd);
		w->OnCharEvent.Fire(k);
	});
	glfwSetMouseButtonCallback((GLFWwindow*)nativeHandle, [](GLFWwindow* wnd, int button, int action, int mods) {
		CWindow* w = GetWindowFromHandle(wnd);
		w->OnMouseButton.Fire((EMouseButton)button, (EInputAction)action, (EInputMod)mods);
	});

	glfwSetWindowSizeCallback((GLFWwindow*)nativeHandle, [](GLFWwindow* window, int width, int height) {
		CWindow* w = GetWindowFromHandle(window);
		w->width = width;
		w->height = height;
		if (w->swapChain && width != 0 && height != 0)
			w->swapChain->Resize(width, height);
		});
	glfwSetWindowFocusCallback((GLFWwindow*)nativeHandle, [](GLFWwindow* window, int focus) { GetWindowFromHandle(window)->bFocused = focus; });
	glfwSetWindowMaximizeCallback((GLFWwindow*)nativeHandle, [](GLFWwindow* window, int b) { GetWindowFromHandle(window)->bIsMaximized = b; });
}

CWindow::~CWindow()
{
	windows.Erase(windows.Find(this));
	glfwDestroyWindow((GLFWwindow*)nativeHandle);
}

void* CWindow::GetNativeHandle()
{
#ifdef _WIN32
	return glfwGetWin32Window(nativeHandle);
#endif
}

void CWindow::SetCursorMode(const ECursorMode& cm)
{
	switch (cm)
	{
	case ECursorMode::NORMAL:
		glfwSetInputMode((GLFWwindow*)nativeHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		break;
	case ECursorMode::HIDDEN:
		glfwSetInputMode((GLFWwindow*)nativeHandle, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		break;
	case ECursorMode::LOCKED:
		glfwSetInputMode((GLFWwindow*)nativeHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		break;
	case ECursorMode::DISABLED:
		glfwSetInputMode((GLFWwindow*)nativeHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		break;
	}
}

void CWindow::SetWindowMode(EWindowMode WindowMode, int targetMonitor)
{
	GLFWmonitor* monitor = nullptr;
	{
		int monitorsCount;
		auto** monitors = glfwGetMonitors(&monitorsCount);
		if (targetMonitor < monitorsCount)
			monitor = monitors[targetMonitor];
	}

	switch (WindowMode)
	{
	case WM_FULLSCREEN:
	{
		if (_WindowMode == WM_WINDOWED)
			UpdateWindowRect();
		const GLFWvidmode* vidMode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor((GLFWwindow*)nativeHandle, monitor, 0, 0, vidMode->width, vidMode->height, vidMode->refreshRate);
	}
		break;
	case WM_WINDOWED:
	{
		glfwSetWindowMonitor((GLFWwindow*)nativeHandle, nullptr, WindowedRect.x, WindowedRect.y, WindowedRect.w, WindowedRect.h, 60);
		glfwSetWindowAttrib((GLFWwindow*)nativeHandle, GLFW_DECORATED, true);
	}
		break;
	case WM_WINDOWED_BORDERLESS:
	{
		if (_WindowMode == WM_WINDOWED)
			UpdateWindowRect();
		const GLFWvidmode* vidMode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor((GLFWwindow*)nativeHandle, nullptr, 0, 0, vidMode->width, vidMode->height, vidMode->refreshRate);
		glfwSetWindowAttrib((GLFWwindow*)nativeHandle, GLFW_DECORATED, false);
	}
		break;
	case WM_WINDOWED_MAXIMIZED:
	{
		glfwSetWindowMonitor((GLFWwindow*)nativeHandle, nullptr, WindowedRect.x, WindowedRect.y, WindowedRect.w, WindowedRect.h, 60);
		glfwSetWindowAttrib((GLFWwindow*)nativeHandle, GLFW_DECORATED, true);
		glfwMaximizeWindow((GLFWwindow*)nativeHandle);
	}
		break;
	}
	_WindowMode = WindowMode;
}

void CWindow::ResizeWindow(int w, int h)
{
	glfwSetWindowSize(nativeHandle, w, h);
}

void CWindow::MoveTo(int x, int y)
{
	glfwSetWindowPos(nativeHandle, x, y);
}

void CWindow::BringToFront()
{
	glfwFocusWindow(nativeHandle);
}

void CWindow::Maximize()
{
	glfwMaximizeWindow(nativeHandle);
}

void CWindow::Show()
{
	glfwShowWindow(nativeHandle);
}

void CWindow::Hide()
{
	glfwHideWindow(nativeHandle);
}

void CWindow::SetWindowTitle(const FString& title)
{
	glfwSetWindowTitle(nativeHandle, title.c_str()); windowTitle = title;
}

bool CWindow::WantsToClose() const
{
	return glfwWindowShouldClose(nativeHandle);
}

void CWindow::Present(int vSync, int flags)
{
	if (swapChain) 
		swapChain->Present(vSync, flags);
}

CWindow* CWindow::GetWindowFromHandle(void* window)
{
	for (auto wnd : windows)
	{
		if (wnd->nativeHandle == window)
			return wnd;
	}
	return nullptr;
}

void CWindow::Init()
{
	THORIUM_ASSERT(glfwInit(), "Failed to initialze glfw");
}

void CWindow::Shutdown()
{
	glfwTerminate();
}
