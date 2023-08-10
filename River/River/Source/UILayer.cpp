#include "RiverPch.h"
#include "UILayer.h"
#include "RiverUI/Header/UIWindow.h"

UILayer::UILayer()
{
}

UILayer::~UILayer()
{
}

void UILayer::OnAttach()
{
	
}

void UILayer::OnDetach()
{
}

void UILayer::OnUpdate(float deltaTime)
{
	for (auto& uiWindow : m_UIWindows)
	{
		uiWindow.second->OnUpdate(deltaTime);
	}
}

bool UILayer::OnEvent(const Event& e)
{
	return false;
}
