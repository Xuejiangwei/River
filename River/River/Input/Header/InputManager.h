#pragma once
#include "RiverHead.h"

class InputManager
{
public:
	InputManager();

	~InputManager();

	Int2 GetLastMousePosition() const { return m_LastMousePositon; }

private:
	Int2 m_LastMousePositon;
};
