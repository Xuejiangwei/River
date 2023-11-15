#pragma once

#include "RiverHead.h"
#include "Renderer/Mesh/Header/Mesh.h"

class Panel;

class UIWindow
{
public:
	UIWindow(Unique<Panel>& rootPanel);

	~UIWindow();

	void OnUpdate(float deltaTime);

	virtual bool OnEvent(const class Event& e);

	void OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices);

	void AddPanel(int orderLevel, Unique<Panel>& panel);

	Panel* GetRootPanel() const { return m_RootPanel.get(); }

private:
	bool OnMouseButtonDown(const class Event& e);

	bool OnMouseButtonRelease(const class Event& e);

private:
	//层级最低
	Unique<Panel> m_RootPanel;

	//同一数组下的Panel，虽然实际层级相同，但越靠后层级越偏高。
	LessRBTreeMap<int, V_Array<Unique<Panel>>> m_Panels;
};
