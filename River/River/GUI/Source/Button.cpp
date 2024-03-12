#include "RiverPch.h"
#include "GUI/Header/Button.h"

Button::Button()
	: m_ClickCall(nullptr), m_MouseButtonClickDetectTime(0.3f), m_MouseButtonClickTimer(0.0f)
{
}

Button::~Button()
{
}

void Button::BindClickFunction(ClickCall func)
{
	//m_ClickCall = func;
}

void Button::OnUpdate(float deltaTime)
{
	if (m_IsMouseButtonDown)
	{
		m_MouseButtonClickTimer += deltaTime;
	}
}

bool Button::OnMouseButtonDown(const Event& e)
{
	m_IsMouseButtonDown = true;
	m_MouseButtonClickTimer = 0.0f;
	return true;
}

bool Button::OnMouseButtonRelease(const Event& e)
{
	m_IsMouseButtonDown = false;
	if (m_MouseButtonClickTimer > m_MouseButtonClickDetectTime)
	{
		auto& releaseEvent = (const MouseButtonReleasedEvent&)e;
		OnMouseButtonClick(releaseEvent.GetMouseX(), releaseEvent.GetMouseY());
	}

	return true;
}

bool Button::OnMouseButtonClick(int mouseX, int mouseY)
{
	if (m_ClickCall)
	{
		m_ClickCall(mouseX, mouseY);
	}
	return true;
}

void Button::AddChildWidget(Share<Widget>&& widget)
{
	m_Child = widget;
}
