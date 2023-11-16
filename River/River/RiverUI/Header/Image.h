#pragma once

#include "RiverHead.h"
#include "Widget.h"

class Image : public Widget, UIMouseEvent
{
public:
	Image();

	virtual ~Image() override;

	virtual void OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices) override;

	virtual bool OnMouseButtonDown(const class Event& e) override;

	virtual bool OnMouseButtonRelease(const class Event& e) override;

	virtual bool OnMouseButtonClick(int mouseX, int mouseY) override { return true; }

	void SetTexture(const char* texturePath);

private:
	const char* m_Texture;
};
