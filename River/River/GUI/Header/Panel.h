#pragma once

#include "RiverHead.h"
#include "Widget.h"

class Image;

// ĿǰPanel����û�в㼶���ֻ�����˳��Խ������㼶Խ�ߣ�Panel����Ҫʵ����Ⱦ����
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

	//ÿ�����ʱ��Ҫ���򣬼��㼶�ߵ���ǰ
	List<Image*> m_MouseButtonDownDetector; 
};
