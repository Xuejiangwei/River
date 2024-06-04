#pragma once
#include "Widget.h"

class InfiniteCanvas : public Widget
{
public:
	InfiniteCanvas();

	~InfiniteCanvas();

	virtual void OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices) override;

	void SetBackgroundColor(uint8 color[4]);

	void SetChildWidgetType(const char* type);

	void SetCenterWidget(Share<Widget> widget) { m_Childs[0] = widget; }

public:
	template<typename T>
	void SetCanvasChildData(V_Array<T> data)
	{
		for (size_t i = 0; i < data.size(); i++)
		{
			auto widget = GuiManager::CreateWidgetByTypeName(m_ChildWidgetType, this);
			m_Childs.push_back(widget);

			auto panel = DynamicCast<Panel>(widget);
			auto text = DynamicCast<Text>(panel->GetChildWidgetByName("TextTitle"));
			text->SetText(data[i]);
		}
	}

public:
	static const String& GetWidgetTypeName()
	{
		static String typeName("InfiniteCanvas");
		return typeName;
	}

private:
	uint8 m_BackgroundColor[4];

	bool m_IsDefaultChildWidgetType;
	String m_ChildWidgetType;
	
	V_Array<Share<Widget>> m_Childs; //��0��Ϊ�������Ŀؼ�
};