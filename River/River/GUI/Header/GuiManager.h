#pragma once
#include "RiverHead.h"

class Widget;
class Panel;

class GuiManager
{
public:
	GuiManager();

	~GuiManager();

	Widget* GetUiWidgetByName(const char* panelName, const char* widgetName);

public:
	static Share<Widget> DecodeGUI_File(const String& filePath);

	static Share<Widget> CreateWidgetByTypeName(const String& typeName, Widget* parent);

public:
	static GuiManager* Get()
	{
		static GuiManager guiManager;
		return &guiManager;
	}

	HashSet<String> m_HazeCodePaths;
};
