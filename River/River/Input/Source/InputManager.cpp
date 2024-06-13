#include "RiverPch.h"
#include "Input/Header/InputManager.h"
#include "Layer.h"

InputManager::InputManager()
{
}

InputManager::~InputManager()
{
}

void InputManager::OnEvent(Event& e, V_Array<Share<Layer>>& layers)
{
	struct Scope
	{
		InputManager* Input;
		Event& E;
		Scope(InputManager* input, Event& e) : Input(input), E(e) {}

		~Scope()
		{
			if (E.GetEventType() == EventType::MouseMoved)
			{
				auto ce = dynamic_cast<MouseMovedEvent&>(E);
				Input->m_LastMousePositon.x = ce.GetMouseX();
				Input->m_LastMousePositon.y = ce.GetMouseY();
			}
		}
	};

	Scope scope(this, e);
	switch (e.GetEventType())
	{
		case EventType::KeyPressed:
		{
			auto& ce = dynamic_cast<KeyPressedEvent&>(e);
			m_KeyState[ce.GetKeyCode()] = KeyState::Press;
		}
			break;
		case EventType::KeyReleased:
		{
			auto& ce = dynamic_cast<KeyReleasedEvent&>(e);
			m_KeyState[ce.GetKeyCode()] = KeyState::Release;
		}
			break;
		case EventType::MouseButtonPressed:
		{
			auto ce = dynamic_cast<MouseButtonPressedEvent&>(e);
			m_MouseState[ce.GetMouseButton()] = MouseState::Press;
			m_LastMousePositon.x = ce.GetMouseFloatX();
			m_LastMousePositon.y = ce.GetMouseFloatY();
		}
			break;
		case EventType::MouseButtonReleased:
		{
			auto ce = dynamic_cast<MouseButtonReleasedEvent&>(e);
			m_MouseState[ce.GetMouseButton()] = MouseState::Release;
			m_LastMousePositon.x = ce.GetMouseFloatX();
			m_LastMousePositon.y = ce.GetMouseFloatY();
		}
			break;
		default:
			break;
	}

	for (int i = (int)layers.size() - 1; i >= 0; i--)
	{
		if (layers[i]->OnEvent(e))
		{
			return;
		}
	}
}

KeyState InputManager::GetKeyState(KeyCode key)
{
	return m_KeyState[key];
}

KeyState InputManager::GetKeyState(V_Array<KeyCode> keys)
{
	KeyState state;
	for (size_t i = 0; i < keys.size(); i++)
	{
		if (i > 0) 
		{
			if (m_KeyState[keys[i]] != state)
			{
				return KeyState::None;
			}
		}
		else
		{
			state = m_KeyState[keys[i]];
		}
	}

	return state;
}

MouseState InputManager::GetMouseState(MouseCode code)
{
	return m_MouseState[code];
}
