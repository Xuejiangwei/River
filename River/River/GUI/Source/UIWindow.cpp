#include "RiverPch.h"
#include "GUI/Header/UIWindow.h"
#include "GUI/Header/Panel.h"

UIWindow::UIWindow(Share<Panel>&& rootPanel)
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
		return OnMouseButtonDown(dynamic_cast<const MouseButtonPressedEvent&>(e));
		break;
	case EventType::MouseButtonReleased:
		return OnMouseButtonRelease(dynamic_cast<const MouseButtonReleasedEvent&>(e));
		break;
	default:
		break;
	}
	
	return false;
}

void UIWindow::OnRender(V_Array<UIVertex>& vertices, V_Array<uint16>& indices)
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

void UIWindow::AddPanel(int orderLevel, Share<Panel>& panel)
{
	m_Panels[orderLevel].push_back(River::Move(panel));
}

Widget* UIWindow::GetWidgetByPanel(const char* panelName, const char* widgetName)
{
	if (m_RootPanel->GetWidgetName() == panelName)
	{
		return m_RootPanel->GetChildWidgetByName(widgetName).get();
	}

	for (auto& panels : m_Panels)
	{
		for (auto& panel : panels.second)
		{
			if (panel->GetWidgetName() == panelName) 
			{
				return panel->GetChildWidgetByName(widgetName).get();
			}
		}
	}

	return nullptr;
}

bool UIWindow::OnMouseButtonDown(const MouseButtonPressedEvent& e)
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

bool UIWindow::OnMouseButtonRelease(const MouseButtonReleasedEvent& e)
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