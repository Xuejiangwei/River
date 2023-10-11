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

	void OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices);

	void AddPanel(int orderLevel, Unique<Panel>& panel);

	Panel* GetRootPanel() const { return m_RootPanel.get(); }

private:
	Unique<Panel> m_RootPanel;
	LessRBTreeMap<int, V_Array<Unique<Panel>>> m_Panels;
};
