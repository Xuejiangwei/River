#pragma once

#include "RiverHead.h"
#include "Widget.h"

// 目前Panel里面没有层级概念，只靠添加顺序，越靠后检测层级越高
class Panel : public Widget
{
public:
	Panel();

	virtual ~Panel() override;

	virtual void OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices) override;

	virtual bool OnMouseButtonDown(const class Event& e);

	virtual bool OnMouseButtonRelease(const class Event& e);

	Panel& operator[](Share<Widget> widget)
	{
		m_Children.push_back(widget);
		return *this;
	}

private:
	bool MouseIsInPanel(int x, int y);

	bool MouseInWidget(Share<Widget>& widget, int x, int y);

private:
	V_Array<Share<Widget>> m_Children;

	//每次添加时需要排序，检测层级高的在前
	V_Array<Share<Widget>> m_MouseButtonDownDetector; 
};
