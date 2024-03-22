#include "RiverPch.h"
#include "GUI/Header/GuiManager.h"
#include "Application.h"
#include "UILayer.h"

GuiManager::GuiManager()
{
}

GuiManager::~GuiManager()
{
}

Widget* GuiManager::GetUiWidgetByName(const char* panelName, const char* widgetName)
{
	return Application::Get()->GetMainUiLayer()->GetUiByName(panelName, widgetName);
}
