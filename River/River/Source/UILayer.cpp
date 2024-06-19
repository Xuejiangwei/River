#include "RiverPch.h"
#include "UILayer.h"
#include "GUI/Header/UIWindow.h"
#include "Utils/Header/StringUtils.h"
#include "GUI/Header/Panel.h"

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

Widget* UILayer::GetUiWidgetByPath(const String& path)
{
	auto names = Split(path, "/");
	if (names.size() > 0)
	{
		Widget* widget = nullptr;
		for (auto& it : m_UIWindows)
		{
			widget = it.second->GetPanel(names[0]);
			if (widget)
			{
				break;
			}
		}

		if (widget)
		{
			if (names.size() == 1)
			{
				return widget;
			}
			else
			{
				for (int i = 1; i < names.size(); i++)
				{
					widget = widget->GetChildWidgetByName(names[i].c_str());
					if (!widget)
					{
						break;
					}
				}

				return widget;
			}
		}
	}

	return nullptr;
}
