#pragma once

#include "RiverHead.h"
#include "RiverUI.h"
#include "Image.h"

class Button : public Image
{
public:
	Button();

	virtual ~Button() override;

	void BindClickFunction(ClickCall func);

	virtual void OnUpdate(float deltaTime) override;

	virtual bool OnMouseButtonDown(const class Event& e) override;

	virtual bool OnMouseButtonRelease(const class Event& e) override;

	virtual bool OnMouseButtonClick(int mouseX, int mouseY) override;

	static const String& GetWidgetTypeName()
	{
		static String typeName("Button");
		return typeName;
	}

private:
	ClickCall m_ClickCall;
	bool m_IsMouseButtonDown;
	float m_MouseButtonClickTimer;
	float m_MouseButtonClickDetectTime;
	
};
