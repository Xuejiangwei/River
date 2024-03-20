#pragma once
#include "RiverHead.h"

class Widget;

class GuiManager
{
public:
	GuiManager();

	~GuiManager();

	Widget* GetUiByName(const char* name);

public:
	static GuiManager* Get()
	{
		static GuiManager guiManager;
		return &guiManager;
	}

private:

};
