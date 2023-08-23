#include "RiverPch.h"
#include "RiverEditorLayer.h"
#include "Application.h"
#include "Renderer/Header/RHI.h"
#include "RiverUI/Header/Panel.h"
#include "RiverUI/Header/Button.h"
#include "RiverUI/Header/Image.h"
#include "RiverUI/Header/Text.h"
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
	rootPanel->SetSize(1, 1);
	rootPanel->SetPosition(0.3f, 0.3f);
	{
		auto text = MakeShare<Text>();
		text->SetSize(1, 1);
		text->SetFontSize(16.0f);
		text->SetText(u8"\u2eec\u2ee6 This.");
		(*rootPanel)[text];
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
	m_RenderVertices.clear();
	m_RenderIndices.clear();
	for (auto& window : m_UIWindows)
	{
		window.second->OnRender(m_RenderVertices, m_RenderIndices);
	}

	RHI::Get()->UpdateUIData(m_RenderVertices, m_RenderIndices);
}
