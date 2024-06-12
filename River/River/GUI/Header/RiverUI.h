#pragma once

#include "RiverHead.h"

using MouseEventCall = Func<void(int mouseButton, int mouseX, int mouseY)>;

class UIMouseEvent 
{
public:
	virtual bool OnMouseButtonDown(const class MouseButtonPressedEvent& e) = 0;
	virtual bool OnMouseButtonRelease(const class MouseButtonReleasedEvent& e) = 0;
	virtual bool OnMouseButtonClick(int mouseButton, int mouseX, int mouseY) = 0;
};
