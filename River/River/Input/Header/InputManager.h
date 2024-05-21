#pragma once
#include "RiverHead.h"
#include "Event.h"
#include "Math/Header/BaseStruct.h"

class Layer;

enum class MouseState
{
	None,
	Press,
	Release,
	Drag
};

enum class KeyState
{
	None,
	Press,
	Release
};

class InputManager
{
public:
	InputManager();

	~InputManager();

	Int2 GetLastMousePosition() const { return m_LastMousePositon; }

	void OnEvent(Event& e, V_Array<Share<Layer>>& layers);

	KeyState GetKeyState(KeyCode key);
	
	KeyState GetKeyState(V_Array<KeyCode> keys);

	MouseState GetMouseState(MouseCode key);

private:
	HashMap<KeyCode, KeyState> m_KeyState;
	HashMap<MouseCode, MouseState> m_MouseState;
	Int2 m_LastMousePositon;
};
