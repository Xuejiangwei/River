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
	RHI::Get()->SetShowUIDebugOutline(true);

	auto rootPanel = MakeUnique<Panel>();
	rootPanel->SetSize(720 / 2, 720 / 2);
	rootPanel->SetPosition(0.0f, 0.0f);
	{
		auto text = MakeShare<Text>();
		text->SetSize(1, 1);
		text->SetFontSize(30.0f);
		text->SetPosition(100, 200);
		text->SetText(u8"齐鸟 This.aabbaabbaabb哈哈哈");
		(*rootPanel)[text];

		auto img = MakeShare<Image>();
		img->SetSize(100, 100);
		img->SetPosition(100, 100);

		(*rootPanel)[img];
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
	RHI::Get()->ClearUIRenderItem();

	for (auto& window : m_UIWindows)
	{
		window.second->OnRender(m_RenderVertices, m_RenderIndices);
	}

	RHI::Get()->UpdateUIData(m_RenderVertices, m_RenderIndices);
}
