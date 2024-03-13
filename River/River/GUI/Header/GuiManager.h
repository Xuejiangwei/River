#pragma once
#include "RiverHead.h"

class GuiManager
{
public:
	GuiManager();

	~GuiManager();


public:
	static GuiManager* Get()
	{
		static GuiManager guiManager;
		return &guiManager;
	}

private:

};
