#pragma once

#include "RiverHead.h"
#include "Widget.h"

// ĿǰPanel����û�в㼶���ֻ�����˳��Խ������㼶Խ��
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

	//ÿ�����ʱ��Ҫ���򣬼��㼶�ߵ���ǰ
	V_Array<Share<Widget>> m_MouseButtonDownDetector; 
};
