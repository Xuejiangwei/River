#pragma once

#include "RiverHead.h"
#include "Widget.h"

class Texture;

//最基本的拥有点击事件的控件
class Image : public Widget, UIMouseEvent
{
public:
	Image();

	virtual ~Image() override;

	virtual void OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices) override;

	virtual bool OnMouseButtonDown(const class MouseButtonPressedEvent& e) override;

	virtual bool OnMouseButtonRelease(const class MouseButtonReleasedEvent& e) override;

	virtual bool OnMouseMove(int mouseX, int mouseY) override;

	virtual bool OnMouseButtonClick(int mouseButton, int mouseX, int mouseY) override { return true; }

	virtual void OnMouseOut() override {}

	static const String& GetWidgetTypeName()
	{
		static String typeName("Image");
		return typeName;
	}

	void SetTexture(Texture* texture);

protected:
	Texture* m_Texture;

	MouseEventCall m_ClickCall;
	MouseEventCall m_MouseDownCall;
	MouseEventCall m_MouseUpCall;
	MouseEventCall m_MouseDragCall;
};
