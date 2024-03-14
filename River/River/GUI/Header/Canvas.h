#pragma once
#include "Widget.h"

class Canvas : public Widget
{
public:
	Canvas();

	~Canvas();

	virtual void OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices) override;

	void SetBackgroundColor(uint8 color[4]);


public:
	static const String& GetWidgetTypeName()
	{
		static String typeName("Canvas");
		return typeName;
	}

private:
	uint8 m_BackgroundColor[4];
};