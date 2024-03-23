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
	static Share<Widget> DecodeGUI_File(const char* filePath);

	static Share<Widget> CreateWidgetByTypeName(const char* typeName);
	
public:
	static GuiManager* Get()
	{
		static GuiManager guiManager;
		return &guiManager;
	}

private:

};
