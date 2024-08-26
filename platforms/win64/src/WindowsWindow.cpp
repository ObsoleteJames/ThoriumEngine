
#include "Platform/Windows/WindowsWindow.h"
#include <Util/Assert.h>

//#define TH_WindowClass "TH_wWindow"
//
//CWindowsWindow::CWindowsWindow(HINSTANCE _hInst, const FString& title, EWindowMode mode, int w, int h, int x, int y) 
//{
//	width = w;
//	height = h;
//
//	hInstance = _hInst;
//
//	bHasFocus = true;
//	bIsMaximized = false;
//	bIsMinimized = false;
//	WindowTitle = title;
//
//	_WindowMode = mode;
//
//	// Register window class.
//	WNDCLASSEX wc;
//	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
//	wc.lpfnWndProc = DefWindowProc;
//	wc.cbClsExtra = 0;
//	wc.cbWndExtra = 0;
//	wc.hInstance = hInstance;
//	wc.hIcon = nullptr;
//	wc.hIconSm = nullptr;
//	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
//	wc.hbrBackground = nullptr;
//	wc.lpszMenuName = nullptr;
//	wc.lpszClassName = TH_WindowClass;
//	wc.cbSize = sizeof(WNDCLASSEX);
//	RegisterClassEx(&wc);
//
//	// Create the window.
//	HWND handle = CreateWindowEx(0, TH_WindowClass, title.c_str(), WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, x, y, width, height, nullptr, nullptr, hInstance, nullptr);
//	nativeHandle = handle;
//
//	THORIUM_ASSERT(handle, "Failed to create window!");
//
//	ShowWindow(handle, SW_SHOW);
//	SetForegroundWindow(handle);
//	SetFocus(handle);
//}
//
//CWindowsWindow::~CWindowsWindow()
//{
//	if (!nativeHandle)
//		return;
//
//	UnregisterClass(TH_WindowClass, hInstance);
//	DestroyWindow((HWND)nativeHandle);
//}
//
//void CWindowsWindow::PollEvents()
//{
//	MSG msg;
//	ZeroMemory(&msg, sizeof(MSG));
//
//	if (PeekMessage(&msg, (HWND)nativeHandle, 0, 0, PM_REMOVE))
//	{
//
//	}
//}
//
//void CWindowsWindow::ResizeWindow(int w, int h)
//{
//
//}
//
//void CWindowsWindow::MoveTo(int x, int y)
//{
//
//}
//
//void CWindowsWindow::BringToFront()
//{
//
//}
//
//void CWindowsWindow::Minimize()
//{
//
//}
//
//void CWindowsWindow::Maximize()
//{
//
//}
//
//void CWindowsWindow::Show()
//{
//
//}
//
//void CWindowsWindow::Hide()
//{
//
//}
//
//void CWindowsWindow::SetWindowMode(EWindowMode WindowMode)
//{
//
//}
