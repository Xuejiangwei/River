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

Widget* GuiManager::GetUiByName(const char* name)
{
	return Application::Get()->GetMainUiLayer()->GetUiByName(name);
}
