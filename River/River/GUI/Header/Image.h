#pragma once

#include "RiverHead.h"
#include "Widget.h"


//�������ӵ�е���¼��Ŀؼ�
class Image : public Widget, UIMouseEvent
{
public:
	Image();

	virtual ~Image() override;

	virtual void OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices) override;

	virtual bool OnMouseButtonDown(const class Event& e) override;

	virtual bool OnMouseButtonRelease(const class Event& e) override;

	virtual bool OnMouseButtonClick(int mouseX, int mouseY) override { return true; }

	static const String& GetWidgetTypeName()
	{
		static String typeName("Image");
		return typeName;
	}

	void SetTexture(Texture* texture);

private:
	Texture* m_Texture;
};
