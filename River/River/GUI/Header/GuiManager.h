#pragma once
#include "RiverHead.h"

class Widget;

class GuiManager
{
public:
	GuiManager();

	~GuiManager();

	Widget* GetUiWidgetByName(const char* panelName, const char* widgetName);

public:
	static GuiManager* Get()
	{
		static GuiManager guiManager;
		return &guiManager;
	}

private:

};
