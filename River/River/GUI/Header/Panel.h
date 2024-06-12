#pragma once

#include "RiverHead.h"
#include "Widget.h"

class Image;

// 目前Panel里面没有层级概念，只靠添加顺序，越靠后检测层级越高，Panel不需要实际渲染数据
class Panel : public Widget
{
public:
	Panel();

	virtual ~Panel() override;

	virtual void OnUpdate(float deltaTime) override;

	virtual void OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices) override;

	virtual bool OnMouseButtonDown(const class MouseButtonPressedEvent& e);

	virtual bool OnMouseButtonRelease(const class MouseButtonReleasedEvent& e);
	
	Share<Widget> GetChildWidgetByName(const char* name);

	Panel& operator[](Share<Widget> widget)
	{
		widget.get()->m_Parent = this;
		m_Children.push_back(widget);
		AddMouseButtonDownDetector(widget.get());

		return *this;
	}

public:
	static const String& GetWidgetTypeName()
	{
		static String typeName("Panel");
		return typeName;
	}
	

private:
	void AddMouseButtonDownDetector(Widget* widget);

	bool MouseIsInPanel(int x, int y);

	bool MouseInWidget(Widget* widget, int x, int y);

private:
	V_Array<Share<Widget>> m_Children;

	//每次添加时需要排序，检测层级高的在前
	List<Image*> m_MouseButtonDownDetector; 
};
