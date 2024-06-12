#pragma once

#include "RiverHead.h"
#include "Renderer/Mesh/Header/Mesh.h"

class Widget;
class Panel;

class UIWindow
{
public:
	UIWindow(Share<Panel>&& rootPanel);

	~UIWindow();

	void OnUpdate(float deltaTime);

	virtual bool OnEvent(const class Event& e);

	void OnRender(V_Array<UIVertex>& vertices, V_Array<uint16>& indices);

	void AddPanel(int orderLevel, Share<Panel>& panel);

	Panel* GetRootPanel() const { return m_RootPanel.get(); }

	Widget* GetWidgetByPanel(const char* panelName, const char* widgetName);

private:
	bool OnMouseButtonDown(const class MouseButtonPressedEvent& e);

	bool OnMouseButtonRelease(const class MouseButtonReleasedEvent& e);

private:
	//�㼶���
	Share<Panel> m_RootPanel;

	//ͬһ�����µ�Panel����Ȼʵ�ʲ㼶��ͬ����Խ����㼶Խƫ�ߡ�
	LessRBTreeMap<int, V_Array<Share<Panel>>> m_Panels;
};
