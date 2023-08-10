#include "RiverPch.h"
#include "RiverUI/Header/UIWindow.h"
#include "RiverUI/Header/Panel.h"

UIWindow::UIWindow(Unique<Panel>& rootPanel)
	: m_RootPanel(std::move(rootPanel))
{
}

UIWindow::~UIWindow()
{
}

void UIWindow::OnUpdate(float deltaTime)
{
	m_RootPanel->OnUpdate(deltaTime);
	for (auto& ps : m_Panels)
	{
		for (auto& p : ps.second)
		{
			p->OnUpdate(deltaTime);
		}
	}
}

void UIWindow::OnRender(V_Array<UIVertex>& vertices, V_Array<uint32_t>& indices)
{
	m_RootPanel->OnRender(vertices, indices);
	for (auto& ps : m_Panels)
	{
		for (auto& p : ps.second)
		{
			p->OnRender(vertices, indices);
		}
	}
}

void UIWindow::AddPanel(int orderLevel, Unique<Panel>& panel)
{
	m_Panels[orderLevel].push_back(std::move(panel));
}