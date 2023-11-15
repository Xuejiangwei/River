#include "RiverPch.h"
#include "RiverUI/Header/UIWindow.h"
#include "RiverUI/Header/Panel.h"

UIWindow::UIWindow(Unique<Panel>& rootPanel)
	: m_RootPanel(std::move(rootPanel))
{
}

UIWindow::~UIWindow()
{
}

void UIWindow::OnUpdate(float deltaTime)
{
	m_RootPanel->OnUpdate(deltaTime);
	for (auto& ps : m_Panels)
	{
		for (auto& p : ps.second)
		{
			p->OnUpdate(deltaTime);
		}
	}
}

bool UIWindow::OnEvent(const class Event& e)
{
	switch (e.GetEventType())
	{
	case EventType::MouseButtonPressed:
		return OnMouseButtonDown(e);
		break;
	case EventType::MouseButtonReleased:
		return OnMouseButtonRelease(e);
		break;
	default:
		break;
	}
	
	return false;
}

void UIWindow::OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices)
{
	m_RootPanel->OnRender(vertices, indices);
	for (auto& ps : m_Panels)
	{
		for (auto& p : ps.second)
		{
			p->OnRender(vertices, indices);
		}
	}
}

void UIWindow::AddPanel(int orderLevel, Unique<Panel>& panel)
{
	m_Panels[orderLevel].push_back(std::move(panel));
}

bool UIWindow::OnMouseButtonDown(const Event& e)
{
	for (auto& it : m_Panels)
	{
		for (int i = (int)it.second.size() - 1; i >= 0; i--)
		{
			if (it.second[i]->OnMouseButtonDown(e))
			{
				return true;
			}
		}
	}
	return m_RootPanel->OnMouseButtonDown(e);
}

bool UIWindow::OnMouseButtonRelease(const Event& e)
{
	for (auto& it : m_Panels)
	{
		for (int i = (int)it.second.size() - 1; i >= 0; i--)
		{
			if (it.second[i]->OnMouseButtonRelease(e))
			{
				return true;
			}
		}
	}
	return m_RootPanel->OnMouseButtonRelease(e);
}