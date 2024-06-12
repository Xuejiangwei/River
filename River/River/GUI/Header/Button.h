#pragma once

#include "RiverHead.h"
#include "RiverUI.h"
#include "Image.h"

class Button : public Image
{
public:
	Button();

	virtual ~Button() override;

	void BindClickFunction(MouseEventCall func);

	virtual void OnUpdate(float deltaTime) override;

	virtual bool OnMouseButtonDown(const class MouseButtonPressedEvent& e) override;

	virtual bool OnMouseButtonRelease(const class MouseButtonReleasedEvent& e) override;

	virtual bool OnMouseButtonClick(int mouseButton, int mouseX, int mouseY) override;

	virtual void AddChildWidget(Share<Widget>&& widget);

	static const String& GetWidgetTypeName()
	{
		static String typeName("Button");
		return typeName;
	}

private:
	MouseEventCall m_ClickCall;
	bool m_IsMouseButtonDown;
	float m_MouseButtonClickTimer;
	float m_MouseButtonClickDetectTime;
	
	Share<Widget> m_Child;
};
