#pragma once

#include "RiverHead.h"
#include "Widget.h"

class Text : public Widget
{
public:
	Text();

	virtual ~Text() override;

	virtual void OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices) override;

	void SetText(const char* text);

	void SetFontSize(float size) { m_FontSize = size; }

private:


private:
	float m_FontSize;
	String m_Text;
};
