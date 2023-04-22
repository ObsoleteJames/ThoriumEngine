
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Window.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

class FEnum_EKeyCode : public FEnum
{
	public:
	FEnum_EKeyCode()
	{
		values.Add({ "UNKOWN", (int64)EKeyCode::UNKOWN });
		values.Add({ "SPACE", (int64)EKeyCode::SPACE });
		values.Add({ "APOSTROPHE", (int64)EKeyCode::APOSTROPHE });
		values.Add({ "COMMA", (int64)EKeyCode::COMMA });
		values.Add({ "MINUS", (int64)EKeyCode::MINUS });
		values.Add({ "PERIOD", (int64)EKeyCode::PERIOD });
		values.Add({ "SLASH", (int64)EKeyCode::SLASH });
		values.Add({ "KEY_0", (int64)EKeyCode::KEY_0 });
		values.Add({ "KEY_1", (int64)EKeyCode::KEY_1 });
		values.Add({ "KEY_2", (int64)EKeyCode::KEY_2 });
		values.Add({ "KEY_3", (int64)EKeyCode::KEY_3 });
		values.Add({ "KEY_4", (int64)EKeyCode::KEY_4 });
		values.Add({ "KEY_5", (int64)EKeyCode::KEY_5 });
		values.Add({ "KEY_6", (int64)EKeyCode::KEY_6 });
		values.Add({ "KEY_7", (int64)EKeyCode::KEY_7 });
		values.Add({ "KEY_8", (int64)EKeyCode::KEY_8 });
		values.Add({ "KEY_9", (int64)EKeyCode::KEY_9 });
		values.Add({ "SEMICOLON", (int64)EKeyCode::SEMICOLON });
		values.Add({ "EQUAL", (int64)EKeyCode::EQUAL });
		values.Add({ "A", (int64)EKeyCode::A });
		values.Add({ "B", (int64)EKeyCode::B });
		values.Add({ "C", (int64)EKeyCode::C });
		values.Add({ "D", (int64)EKeyCode::D });
		values.Add({ "E", (int64)EKeyCode::E });
		values.Add({ "F", (int64)EKeyCode::F });
		values.Add({ "G", (int64)EKeyCode::G });
		values.Add({ "H", (int64)EKeyCode::H });
		values.Add({ "I", (int64)EKeyCode::I });
		values.Add({ "J", (int64)EKeyCode::J });
		values.Add({ "K", (int64)EKeyCode::K });
		values.Add({ "L", (int64)EKeyCode::L });
		values.Add({ "M", (int64)EKeyCode::M });
		values.Add({ "N", (int64)EKeyCode::N });
		values.Add({ "O", (int64)EKeyCode::O });
		values.Add({ "P", (int64)EKeyCode::P });
		values.Add({ "Q", (int64)EKeyCode::Q });
		values.Add({ "R", (int64)EKeyCode::R });
		values.Add({ "S", (int64)EKeyCode::S });
		values.Add({ "T", (int64)EKeyCode::T });
		values.Add({ "U", (int64)EKeyCode::U });
		values.Add({ "V", (int64)EKeyCode::V });
		values.Add({ "W", (int64)EKeyCode::W });
		values.Add({ "X", (int64)EKeyCode::X });
		values.Add({ "Y", (int64)EKeyCode::Y });
		values.Add({ "Z", (int64)EKeyCode::Z });
		values.Add({ "LEFT_BRACKET", (int64)EKeyCode::LEFT_BRACKET });
		values.Add({ "BACKSLASH", (int64)EKeyCode::BACKSLASH });
		values.Add({ "RIGHT_BRACKET", (int64)EKeyCode::RIGHT_BRACKET });
		values.Add({ "GRAVE_ACCENT", (int64)EKeyCode::GRAVE_ACCENT });
		values.Add({ "WORLD_1", (int64)EKeyCode::WORLD_1 });
		values.Add({ "WORLD_2", (int64)EKeyCode::WORLD_2 });
		values.Add({ "ESCAPE", (int64)EKeyCode::ESCAPE });
		values.Add({ "ENTER", (int64)EKeyCode::ENTER });
		values.Add({ "TAB", (int64)EKeyCode::TAB });
		values.Add({ "BACKSPACE", (int64)EKeyCode::BACKSPACE });
		values.Add({ "INSERT", (int64)EKeyCode::INSERT });
		values.Add({ "KEY_DELETE", (int64)EKeyCode::KEY_DELETE });
		values.Add({ "RIGHT", (int64)EKeyCode::RIGHT });
		values.Add({ "LEFT", (int64)EKeyCode::LEFT });
		values.Add({ "DOWN", (int64)EKeyCode::DOWN });
		values.Add({ "UP", (int64)EKeyCode::UP });
		values.Add({ "PAGE_UP", (int64)EKeyCode::PAGE_UP });
		values.Add({ "PAGE_DOWN", (int64)EKeyCode::PAGE_DOWN });
		values.Add({ "HOME", (int64)EKeyCode::HOME });
		values.Add({ "END", (int64)EKeyCode::END });
		values.Add({ "CAPS_LOCK", (int64)EKeyCode::CAPS_LOCK });
		values.Add({ "SCROLL_LOCK", (int64)EKeyCode::SCROLL_LOCK });
		values.Add({ "NUM_LOCK", (int64)EKeyCode::NUM_LOCK });
		values.Add({ "PRINT_SCREEN", (int64)EKeyCode::PRINT_SCREEN });
		values.Add({ "PAUSE", (int64)EKeyCode::PAUSE });
		values.Add({ "F1", (int64)EKeyCode::F1 });
		values.Add({ "F2", (int64)EKeyCode::F2 });
		values.Add({ "F3", (int64)EKeyCode::F3 });
		values.Add({ "F4", (int64)EKeyCode::F4 });
		values.Add({ "F5", (int64)EKeyCode::F5 });
		values.Add({ "F6", (int64)EKeyCode::F6 });
		values.Add({ "F7", (int64)EKeyCode::F7 });
		values.Add({ "F8", (int64)EKeyCode::F8 });
		values.Add({ "F9", (int64)EKeyCode::F9 });
		values.Add({ "F10", (int64)EKeyCode::F10 });
		values.Add({ "F11", (int64)EKeyCode::F11 });
		values.Add({ "F12", (int64)EKeyCode::F12 });
		values.Add({ "F13", (int64)EKeyCode::F13 });
		values.Add({ "F14", (int64)EKeyCode::F14 });
		values.Add({ "F15", (int64)EKeyCode::F15 });
		values.Add({ "F16", (int64)EKeyCode::F16 });
		values.Add({ "F17", (int64)EKeyCode::F17 });
		values.Add({ "F18", (int64)EKeyCode::F18 });
		values.Add({ "F19", (int64)EKeyCode::F19 });
		values.Add({ "F20", (int64)EKeyCode::F20 });
		values.Add({ "F21", (int64)EKeyCode::F21 });
		values.Add({ "F22", (int64)EKeyCode::F22 });
		values.Add({ "F23", (int64)EKeyCode::F23 });
		values.Add({ "F24", (int64)EKeyCode::F24 });
		values.Add({ "F25", (int64)EKeyCode::F25 });
		values.Add({ "KP_0", (int64)EKeyCode::KP_0 });
		values.Add({ "KP_1", (int64)EKeyCode::KP_1 });
		values.Add({ "KP_2", (int64)EKeyCode::KP_2 });
		values.Add({ "KP_3", (int64)EKeyCode::KP_3 });
		values.Add({ "KP_4", (int64)EKeyCode::KP_4 });
		values.Add({ "KP_5", (int64)EKeyCode::KP_5 });
		values.Add({ "KP_6", (int64)EKeyCode::KP_6 });
		values.Add({ "KP_7", (int64)EKeyCode::KP_7 });
		values.Add({ "KP_8", (int64)EKeyCode::KP_8 });
		values.Add({ "KP_9", (int64)EKeyCode::KP_9 });
		values.Add({ "KP_DECIMAL", (int64)EKeyCode::KP_DECIMAL });
		values.Add({ "KP_DIVIDE", (int64)EKeyCode::KP_DIVIDE });
		values.Add({ "KP_MULTIPLY", (int64)EKeyCode::KP_MULTIPLY });
		values.Add({ "KP_SUBTRACT", (int64)EKeyCode::KP_SUBTRACT });
		values.Add({ "KP_ADD", (int64)EKeyCode::KP_ADD });
		values.Add({ "KP_ENTER", (int64)EKeyCode::KP_ENTER });
		values.Add({ "KP_EQUAL", (int64)EKeyCode::KP_EQUAL });
		values.Add({ "LEFT_SHIFT", (int64)EKeyCode::LEFT_SHIFT });
		values.Add({ "LEFT_CONTROL", (int64)EKeyCode::LEFT_CONTROL });
		values.Add({ "LEFT_ALT", (int64)EKeyCode::LEFT_ALT });
		values.Add({ "LEFT_SUPER", (int64)EKeyCode::LEFT_SUPER });
		values.Add({ "RIGHT_SHIFT", (int64)EKeyCode::RIGHT_SHIFT });
		values.Add({ "RIGHT_CONTROL", (int64)EKeyCode::RIGHT_CONTROL });
		values.Add({ "RIGHT_ALT", (int64)EKeyCode::RIGHT_ALT });
		values.Add({ "RIGHT_SUPER", (int64)EKeyCode::RIGHT_SUPER });
		values.Add({ "MENU", (int64)EKeyCode::MENU });
		name = "KeyCode";
		cppName = "EKeyCode";
		size = sizeof(EKeyCode);
		flags = EnumFlag_NONE;
		GetModule_Engine().RegisterFEnum(this);
	}
};
FEnum_EKeyCode __FEnum_EKeyCode_Instance;

class FEnum_EMouseButton : public FEnum
{
	public:
	FEnum_EMouseButton()
	{
		values.Add({ "LEFT", (int64)EMouseButton::LEFT });
		values.Add({ "RIGHT", (int64)EMouseButton::RIGHT });
		values.Add({ "MIDDLE", (int64)EMouseButton::MIDDLE });
		values.Add({ "MOUSE4", (int64)EMouseButton::MOUSE4 });
		values.Add({ "MOUSE5", (int64)EMouseButton::MOUSE5 });
		values.Add({ "MOUSE6", (int64)EMouseButton::MOUSE6 });
		values.Add({ "MOUSE7", (int64)EMouseButton::MOUSE7 });
		values.Add({ "MOUSE8", (int64)EMouseButton::MOUSE8 });
		values.Add({ "NONE", (int64)EMouseButton::NONE });
		name = "MouseButton";
		cppName = "EMouseButton";
		size = sizeof(EMouseButton);
		flags = EnumFlag_NONE;
		GetModule_Engine().RegisterFEnum(this);
	}
};
FEnum_EMouseButton __FEnum_EMouseButton_Instance;

