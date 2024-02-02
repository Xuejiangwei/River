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
	EventDispatcher dispatcher(e);

	//Mouse
	dispatcher.DispatchDirect<MouseButtonPressedEvent>(
		[this, &layers](auto e) -> decltype(auto)
		{
			auto& ce = dynamic_cast<MouseButtonPressedEvent&>(e);
			m_MouseState[ce.GetMouseButton()] = MouseState::Press;
			m_LastMousePositon.x = ce.GetMouseX();
			m_LastMousePositon.y = ce.GetMouseY();

			for (int i = (int)layers.size() - 1; i >= 0; i--)
			{
				if (layers[i]->OnMousePress(ce.GetMouseButton(), m_LastMousePositon))
				{
					return;
				}
			}
		});

	dispatcher.DispatchDirect<MouseButtonReleasedEvent>(
		[this, &layers](auto e) -> decltype(auto)
		{
			auto& ce = dynamic_cast<MouseButtonReleasedEvent&>(e);
			m_MouseState[ce.GetMouseButton()] = MouseState::Release;
			m_LastMousePositon.x = ce.GetMouseX();
			m_LastMousePositon.y = ce.GetMouseY();

			for (int i = (int)layers.size() - 1; i >= 0; i--)
			{
				if (layers[i]->OnMouseRelease(ce.GetMouseButton(), m_LastMousePositon))
				{
					return;
				}
			}
		});

	dispatcher.DispatchDirect<MouseMovedEvent>(
		[this, &layers](auto e) -> decltype(auto)
		{
			bool isDrag = false;
			auto& ce = dynamic_cast<MouseMovedEvent&>(e);
			for (auto& state : m_MouseState)
			{
				if (state.second == MouseState::Press)
				{
					state.second = MouseState::Drag;
					isDrag = true;
				}
			}

			m_LastMousePositon.x = (int)ce.GetMouseX();
			m_LastMousePositon.y = (int)ce.GetMouseY();

			if (isDrag)
			{
				for (int i = (int)layers.size() - 1; i >= 0; i--)
				{
					if (layers[i]->OnMouseMove(m_LastMousePositon.x, m_LastMousePositon.y))
					{
						return;
					}
				}
			}
		});


	//Key code
	dispatcher.DispatchDirect<KeyPressedEvent>(
		[this, &layers](auto e) -> decltype(auto)
		{
			auto& ce = dynamic_cast<KeyPressedEvent&>(e);
			m_KeyState[ce.GetKeyCode()] = KeyState::Press;
			for (int i = (int)layers.size() - 1; i >= 0; i--)
			{
				if (layers[i]->OnKeyPress(ce.GetKeyCode()))
				{
					return;
				}
			}
		});

	dispatcher.DispatchDirect<KeyReleasedEvent>(
		[this, &layers](auto e) -> decltype(auto)
		{
			auto& ce = dynamic_cast<KeyReleasedEvent&>(e);
			m_KeyState[ce.GetKeyCode()] = KeyState::Release;

			for (int i = (int)layers.size() - 1; i >= 0; i--)
			{
				if (layers[i]->OnKeyRelease(ce.GetKeyCode()))
				{
					return;
				}
			}

			m_KeyState[ce.GetKeyCode()] = KeyState::None;
		});
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
