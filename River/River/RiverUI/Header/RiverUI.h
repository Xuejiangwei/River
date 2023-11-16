#pragma once

#include "RiverHead.h"

using ClickCall = Func<void(int mouseX, int mouseY)>;

class UIMouseEvent 
{
public:
	virtual bool OnMouseButtonDown(const class Event& e) = 0;
	virtual bool OnMouseButtonRelease(const class Event& e) = 0;
	virtual bool OnMouseButtonClick(int mouseX, int mouseY) = 0;
};
