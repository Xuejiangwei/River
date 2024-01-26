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
		[this, layers](auto e) -> decltype(auto)
		{
			auto& ce = dynamic_cast<MouseButtonPressedEvent&>(e);
			m_MouseState[ce.GetMouseButton()] = MouseState::Press;
			m_LastMousePositon.x = ce.GetMouseX();
			m_LastMousePositon.y = ce.GetMouseY();

			for (auto i = layers.size() - 1; i >= 0; i--)
			{
				/*if (layers[i]->)
				{

				}*/
			}
		});

	dispatcher.DispatchDirect<MouseButtonReleasedEvent>(
		[this](auto e) -> decltype(auto)
		{
			auto& ce = dynamic_cast<MouseButtonReleasedEvent&>(e);
			m_MouseState[ce.GetMouseButton()] = MouseState::Release;
			m_LastMousePositon.x = ce.GetMouseX();
			m_LastMousePositon.y = ce.GetMouseY();
		});

	dispatcher.DispatchDirect<MouseMovedEvent>(
		[this](auto e) -> decltype(auto)
		{
			auto& ce = dynamic_cast<MouseMovedEvent&>(e);
			for (auto& state : m_MouseState)
			{
				if (state.second == MouseState::Press)
				{
					state.second = MouseState::Drag;
				}
			}
			m_LastMousePositon.x = ce.GetMouseX();
			m_LastMousePositon.y = ce.GetMouseY();
		});


	//Key code
	dispatcher.DispatchDirect<KeyPressedEvent>(
		[this](auto e) -> decltype(auto)
		{
			auto& ce = dynamic_cast<KeyPressedEvent&>(e);
			//RHI::Get()->GetMainCamera()->OnKeyPressed(ce.GetKeyCode(), m_Time);
		});

	dispatcher.DispatchDirect<KeyPressedEvent>(
		[this](auto e) -> decltype(auto)
		{
			auto& ce = dynamic_cast<KeyPressedEvent&>(e);
			//RHI::Get()->GetMainCamera()->OnKeyPressed(ce.GetKeyCode(), m_Time);
		});
}
