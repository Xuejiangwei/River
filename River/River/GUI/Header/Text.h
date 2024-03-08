#pragma once

#include "RiverHead.h"
#include "Widget.h"

class Texture;

class Text : public Widget
{
public:
	Text();

	virtual ~Text() override;

	virtual void OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices) override;

	static const String& GetWidgetTypeName()
	{
		static String typeName("Text");
		return typeName;
	}

	void SetFont(Texture* font);

	void SetText(const char* text);

	void SetFontSize(float size) { m_FontSize = size; }

private:
	Texture* m_Font;
	float m_FontSize;
	String m_Text;
};
