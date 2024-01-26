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

class InputManager
{
public:
	InputManager();

	~InputManager();

	Int2 GetLastMousePosition() const { return m_LastMousePositon; }

	void OnEvent(Event& e, V_Array<Share<Layer>>& layers);

private:
	HashSet<KeyCode> m_PressKeys;
	HashMap<MouseCode, MouseState> m_MouseState;
	Int2 m_LastMousePositon;
};
