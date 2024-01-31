#include "RiverPch.h"
#include "RiverEditorMenuLayer.h"
#include "Application.h"
#include "Renderer/Header/RHI.h"
#include "Renderer/Header/AssetManager.h"
#include "Renderer/Header/RenderScene.h"
#include "Renderer/Pass/Header/RenderPassUI.h"
#include "RiverUI/Header/Panel.h"
#include "RiverUI/Header/Button.h"
#include "RiverUI/Header/Image.h"
#include "RiverUI/Header/Text.h"
#include "RiverUI/Header/UIWindow.h"

RiverEditorMenuLayer::RiverEditorMenuLayer()
{
}

RiverEditorMenuLayer::~RiverEditorMenuLayer()
{
}

void RiverEditorMenuLayer::OnInitialize()
{
}

void RiverEditorMenuLayer::OnAttach()
{
	//initialize
	RHI::Get()->SetShowUIDebugOutline(false);

	auto rootPanel = MakeUnique<Panel>();
	rootPanel->SetSize(720 / 2, 720 / 2);
	rootPanel->SetPosition(0.0f, 0.0f);
	{
		/*auto text = MakeShare<Text>();
		text->SetSize(1, 1);
		text->SetFont(AssetManager::Get()->GetTexture("font"));
		text->SetFontSize(30.0f);
		text->SetPosition(100, 200);
		text->SetText(u8"齐鸟 This.aabbaabbaabb哈哈哈");
		(*rootPanel)[text];

		auto img = MakeShare<Image>();
		img->SetTexture(AssetManager::Get()->GetTexture("bricksNormalMap"));
		img->SetSize(100, 100);
		img->SetPosition(100, 100);

		(*rootPanel)[img];*/

		auto img1 = MakeShare<Image>();
		img1->SetTexture(AssetManager::Get()->GetTexture("tileNormalMap"));
		img1->SetSize(360, 360);
		img1->SetPosition(300, 300);

		(*rootPanel)[img1];
	}

	m_UIWindows[0] = MakeUnique<UIWindow>(rootPanel);
}

void RiverEditorMenuLayer::OnDetach()
{

}

void RiverEditorMenuLayer::OnUpdate(float deltaTime)
{

}

bool RiverEditorMenuLayer::OnEvent(const Event& e)
{
	for (auto& it : m_UIWindows)
	{
		if (it.second->OnEvent(e))
		{
			return true;
		}

	}
	return false;
}

void RiverEditorMenuLayer::OnRender()
{
	auto uiRenderPass = Application::Get()->GetRenderScene()->GetUIRenderPass();
	auto& renderVertices = uiRenderPass->GetRenderVertices();
	auto& renderIndices = uiRenderPass->GetRenderIndices();

	for (auto& window : m_UIWindows)
	{
		window.second->OnRender(renderVertices, renderIndices);
	}
}
