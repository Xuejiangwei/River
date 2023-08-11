#include "RiverPch.h"
#include "RiverEditorLayer.h"
#include "Application.h"
#include "Renderer/Header/RHI.h"
#include "RiverUI/Header/Panel.h"
#include "RiverUI/Header/Button.h"
#include "RiverUI/Header/Image.h"
#include "RiverUI/Header/UIWindow.h"

RiverEditorLayer::RiverEditorLayer()
{
}

RiverEditorLayer::~RiverEditorLayer()
{
}

void RiverEditorLayer::OnAttach()
{
	//initialize
	auto rootPanel = MakeUnique<Panel>();
	
	{
		auto testImage = MakeShare<Image>();
		testImage->SetSize(100, 100);
		(*rootPanel)[testImage];
	}

	{
		auto testButton = MakeShare<Button>();
		testButton->SetSize(100, 100);
		(*rootPanel)[testButton];
	}

	m_UIWindows[0] = MakeUnique<UIWindow>(rootPanel);
}

void RiverEditorLayer::OnDetach()
{

}

void RiverEditorLayer::OnUpdate(float deltaTime)
{

}

bool RiverEditorLayer::OnEvent(const Event& e)
{
	return false;
}

void RiverEditorLayer::OnRender()
{
	for (auto& window : m_UIWindows)
	{
		window.second->OnRender(m_RenderVertices, m_RenderIndices);
	}

	RHI::Get()->UpdateUIData(m_RenderVertices, m_RenderIndices);
}
