#pragma once
#include "Image.h"

class InfiniteCanvas : public Image
{
public:
	InfiniteCanvas();

	~InfiniteCanvas();

	virtual void OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices) override;
	
	virtual bool OnMouseButtonDown(const class MouseButtonPressedEvent& e) override;
	
	virtual bool OnMouseButtonRelease(const class MouseButtonReleasedEvent& e) override;

	virtual bool OnMouseMove(int mouseX, int mouseY) override;

	virtual void OnMouseOut() override;

	virtual Float2 GetAbsoluteLeftTopPosition() override;

	virtual Widget* GetChildWidgetByName(const char* name);

	void SetBackgroundColor(uint8 color[4]);

	void SetChildWidgetType(const char* type);

	void SetCenterWidget(Share<Widget> widget) { m_Childs[0] = widget; }

	void CreateChildWidget(int count);

public:
	/*template<typename T>
	void SetCanvasChildData(V_Array<T> data)
	{
		for (size_t i = 0; i < data.size(); i++)
		{
			auto widget = GuiManager::CreateWidgetByTypeName(m_ChildWidgetType, this);
			m_Childs.push_back(widget);

			auto panel = DynamicCast<Panel>(widget);
			auto size = panel->GetSize();
			panel->SetPosition(i * size.x, i * size.y);
			auto text = dynamic_cast<Text*>(panel->GetChildWidgetByName("TextTitle"));
			text->SetText(data[i]);
		}
	}*/

public:
	static const String& GetWidgetTypeName()
	{
		static String typeName("InfiniteCanvas");
		return typeName;
	}

private:
	uint8 m_BackgroundColor[4];
	bool m_IsMousePressed;
	bool m_IsDefaultChildWidgetType;
	
	Float2 m_MovedPosition;
	String m_ChildWidgetType;
	
	V_Array<Share<Widget>> m_Childs; //第0个为索引中心控件
};