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
				if (layers[i]->OnMousePress())
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
				if (layers[i]->OnMouseRelease())
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
			m_LastMousePositon.x = ce.GetMouseX();
			m_LastMousePositon.y = ce.GetMouseY();

			if (isDrag)
			{
				for (int i = (int)layers.size() - 1; i >= 0; i--)
				{
					if (layers[i]->OnMouseDrag())
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
			
			for (int i = (int)layers.size() - 1; i >= 0; i--)
			{
				if (layers[i]->OnKeyPress())
				{
					return;
				}
			}
		});

	dispatcher.DispatchDirect<KeyReleasedEvent>(
		[this, &layers](auto e) -> decltype(auto)
		{
			auto& ce = dynamic_cast<KeyReleasedEvent&>(e);

			for (int i = (int)layers.size() - 1; i >= 0; i--)
			{
				if (layers[i]->OnKeyRelease())
				{
					return;
				}
			}
		});
}
