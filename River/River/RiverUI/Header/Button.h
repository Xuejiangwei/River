#pragma once

#include "RiverHead.h"
#include "RiverUI.h"
#include "Widget.h"

class Button : public Widget, UIMouseEvent
{
public:
	Button();

	virtual ~Button() override;

	void BindClickFunction(ClickCall func);

	virtual void OnUpdate(float deltaTime) override;

	virtual bool OnMouseButtonDown(const class Event& e) override;

	virtual bool OnMouseButtonRelease(const class Event& e) override;

	virtual bool OnMouseButtonClick(int mouseX, int mouseY) override;

private:
	ClickCall m_ClickCall;
	bool m_IsMouseButtonDown;
	float m_MouseButtonClickTimer;
	float m_MouseButtonClickDetectTime;
	
};
