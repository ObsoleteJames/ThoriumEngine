#pragma once

#include <Util/Event.h>
#include "EngineCore.h"
#include "Object/Object.h"
#include "Object/Delegate.h"
#include "Window.generated.h"

struct GLFWwindow;
class ISwapChain;

ENUM()
enum class EKeyCode
{
	UNKOWN = -1,
	SPACE = 32,
	APOSTROPHE = 39,
	COMMA = 44,
	MINUS = 45,
	PERIOD = 46,
	SLASH = 47,
	KEY_0 = 48,
	KEY_1 = 49,
	KEY_2 = 50,
	KEY_3 = 51,
	KEY_4 = 52,
	KEY_5 = 53,
	KEY_6 = 54,
	KEY_7 = 55,
	KEY_8 = 56,
	KEY_9 = 57,
	SEMICOLON = 59,
	EQUAL = 61,
	A = 65,
	B = 66,
	C = 67,
	D = 68,
	E = 69,
	F = 70,
	G = 71,
	H = 72,
	I = 73,
	J = 74,
	K = 75,
	L = 76,
	M = 77,
	N = 78,
	O = 79,
	P = 80,
	Q = 81,
	R = 82,
	S = 83,
	T = 84,
	U = 85,
	V = 86,
	W = 87,
	X = 88,
	Y = 89,
	Z = 90,
	LEFT_BRACKET = 91,
	BACKSLASH = 92,
	RIGHT_BRACKET = 93,
	GRAVE_ACCENT = 96,
	WORLD_1 = 161,
	WORLD_2 = 162,
	ESCAPE = 256,
	ENTER = 257,
	TAB = 258,
	BACKSPACE = 259,
	INSERT = 260,
	KEY_DELETE = 261,
	RIGHT = 262,
	LEFT = 263,
	DOWN = 264,
	UP = 265,
	PAGE_UP = 266,
	PAGE_DOWN = 267,
	HOME = 268,
	END = 269,
	CAPS_LOCK = 280,
	SCROLL_LOCK = 281,
	NUM_LOCK = 282,
	PRINT_SCREEN = 283,
	PAUSE = 284,
	F1 = 290,
	F2 = 291,
	F3 = 292,
	F4 = 293,
	F5 = 294,
	F6 = 295,
	F7 = 296,
	F8 = 297,
	F9 = 298,
	F10 = 299,
	F11 = 300,
	F12 = 301,
	F13 = 302,
	F14 = 303,
	F15 = 304,
	F16 = 305,
	F17 = 306,
	F18 = 307,
	F19 = 308,
	F20 = 309,
	F21 = 310,
	F22 = 311,
	F23 = 312,
	F24 = 313,
	F25 = 314,
	KP_0 = 320,
	KP_1 = 321,
	KP_2 = 322,
	KP_3 = 323,
	KP_4 = 324,
	KP_5 = 325,
	KP_6 = 326,
	KP_7 = 327,
	KP_8 = 328,
	KP_9 = 329,
	KP_DECIMAL = 330,
	KP_DIVIDE = 331,
	KP_MULTIPLY = 332,
	KP_SUBTRACT = 333,
	KP_ADD = 334,
	KP_ENTER = 335,
	KP_EQUAL = 336,
	LEFT_SHIFT = 340,
	LEFT_CONTROL = 341,
	LEFT_ALT = 342,
	LEFT_SUPER = 343,
	RIGHT_SHIFT = 344,
	RIGHT_CONTROL = 345,
	RIGHT_ALT = 346,
	RIGHT_SUPER = 347,
	MENU = 348
};

enum class ECursorMode
{
	NORMAL,
	HIDDEN,
	LOCKED, // Cursor is locked within the window.
	DISABLED,
};

ENUM()
enum class EMouseButton
{
	LEFT,
	RIGHT,
	MIDDLE,
	MOUSE4,
	MOUSE5,
	MOUSE6,
	MOUSE7,
	MOUSE8,
	NONE
};

enum EInputAction
{
	IE_RELEASE,
	IE_PRESS,
	IE_REPEAT
};

enum EInputMod
{
	IM_NONE = 0,
	IM_SHIFT = 1,
	IM_CONTROL = 1 << 1,
	IM_ALT = 1 << 2,
	IM_SUPER = 1 << 3,
	IM_CAPS_LOCK = 1 << 4,
	IM_NUM_LOCK = 1 << 5
};

class ENGINE_API IBaseWindow
{
public:
	virtual void* GetNativeHandle() = 0;
	inline void GetSize(int& w, int& h) const { w = width; h = height; }
	inline void GetMousePos(double& x, double& y) const { x = mouseX; y = mouseY; }

	virtual void SetCursorMode(const ECursorMode& cm) {}

public:
	TDelegate<EKeyCode, EInputAction, EInputMod> OnKeyEvent;
	TDelegate<uint> OnCharEvent;

	TDelegate<double, double> OnCursorMove; // X, Y
	TDelegate<EMouseButton, EInputAction, EInputMod> OnMouseButton;

protected:
	double mouseX, mouseY;
	int width, height;

};

class ENGINE_API CWindow : public IBaseWindow
{
public:
	enum EWindowMode
	{
		WM_FULLSCREEN,
		WM_WINDOWED,
		WM_WINDOWED_BORDERLESS,
		WM_WINDOWED_MAXIMIZED
	};

	friend class CEngine;

public:
	CWindow(int w, int h, int x, int y, const FString& title, EWindowMode mode = WM_WINDOWED);
	~CWindow();

	virtual void* GetNativeHandle();

	virtual void SetCursorMode(const ECursorMode& cm);

	void SetWindowMode(EWindowMode WindowMode, int targetMonitor = 0);
	
	void ResizeWindow(int w, int h);
	void MoveTo(int x, int y);
	void BringToFront();
	void Maximize();
	inline void Minimize() { Hide(); }
	void Show();
	void Hide();
	void SetWindowTitle(const FString& title);
	bool WantsToClose() const;

	inline ISwapChain* GetSwapChain() const { return swapChain; }

	inline const FString& GetWindowTitle() const { return windowTitle; }
	inline EWindowMode GetWindowMode() const { return bIsMaximized ? WM_WINDOWED_MAXIMIZED : _WindowMode; }
	inline bool IsMaximized() const { return bIsMaximized; }
	inline bool IsMinimized() const { return bIsMinimized; }
	inline bool IsFocused() const { return bFocused; }
	inline bool IsFullscreen() const { return _WindowMode == WM_FULLSCREEN; }

	void Present(int vSync, int flags);

	static CWindow* GetWindowFromHandle(void* window);

	static void Init();
	static void Shutdown();

protected:
	static void PollEvents();

	void UpdateWindowRect();

protected:
	bool bIsMaximized;
	bool bIsMinimized;
	bool bFocused;

	struct {
		int x, y;
		int w = 1920, h = 1080;
	} WindowedRect;

	FString windowTitle;

	EWindowMode _WindowMode;
	GLFWwindow* nativeHandle;

	ISwapChain* swapChain;

	static TArray<CWindow*> windows;
};
