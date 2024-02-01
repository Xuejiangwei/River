#pragma once
#include "RiverHead.h"
#include "Event.h"

class Layer;

enum class MouseState
{
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

	KeyState GetKeyState(V_Array<KeyCode> keys);

private:
	HashMap<KeyCode, KeyState> m_KeyState;
	HashMap<MouseCode, MouseState> m_MouseState;
	Int2 m_LastMousePositon;
};
