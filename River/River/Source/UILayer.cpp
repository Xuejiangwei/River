#include "RiverPch.h"
#include "UILayer.h"
#include "GUI/Header/UIWindow.h"

UILayer::UILayer()
{
}

UILayer::~UILayer()
{
}

void UILayer::OnInitialize()
{
}

void UILayer::OnAttach()
{
	
}

void UILayer::OnDetach()
{
}

void UILayer::OnUpdate(float deltaTime)
{
	for (auto& uiWindow : m_UIWindows)
	{
		uiWindow.second->OnUpdate(deltaTime);
	}
}

bool UILayer::OnEvent(const Event& e)
{
	for (auto& it : m_UIWindows)
	{
		if (it.second->OnEvent(e))
		{
			return true;
		}

	}
	return false;
}

Widget* UILayer::GetUiByName(const char* panelName, const char* widgetName)
{
	Widget* widget = nullptr;
	for (auto& it : m_UIWindows)
	{
		widget = it.second->GetWidgetByPanel(panelName, widgetName);
		if (widget)
		{
			break;
		}
	}

	return widget;
}
