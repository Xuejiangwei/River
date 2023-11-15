#pragma once

#include "RiverHead.h"
#include "RiverUI.h"
#include "Widget.h"

class Button : public Widget
{
public:
	Button();

	virtual ~Button() override;

	void BindClickFunction(ClickCall func);

	virtual bool OnMouseButtonDown(const class Event& e);

private:
	ClickCall m_ClickCall;
};
