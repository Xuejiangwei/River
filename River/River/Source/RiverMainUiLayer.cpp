#include "RiverPch.h"
#include "RiverMainUiLayer.h"

#include "Application.h"
#include "Renderer/Header/RHI.h"
#include "Renderer/Header/AssetManager.h"
#include "Renderer/Header/RenderScene.h"
#include "Renderer/Pass/Header/RenderPassUI.h"
#include "GUI/Header/GuiDecode.h"
#include "GUI/Header/GuiDecode.h"
#include "GUI/Header/Panel.h"
#include "GUI/Header/Button.h"
#include "GUI/Header/Image.h"
#include "GUI/Header/Text.h"
#include "GUI/Header/Canvas.h"
#include "GUI/Header/ListWidget.h"
#include "GUI/Header/UIWindow.h"

RiverMainUiLayer::RiverMainUiLayer()
{
}

RiverMainUiLayer::~RiverMainUiLayer()
{
}

void RiverMainUiLayer::OnInitialize()
{
}

void RiverMainUiLayer::OnAttach()
{
	//initialize
	RHI::Get()->SetShowUIDebugOutline(true);
	auto panel = DynamicCast<Panel>(DecodeGUI_File("F:\\GitHub\\River\\River\\UI\\MainUI.json"));
	/*auto list = DynamicCast<ListWidget>(panel->GetChildWidgetByName("MainList"));
	list->SetChildWidgetType("Text");
	list->SetListData<String>({ String(u8"од"), String(u8"ил"), String(u8"жэ") });

	auto canvas = DynamicCast<Canvas>(panel->GetChildWidgetByName("MainCanvas"));
	uint8 color[4] = { 0, 255, 255, 255 };
	canvas->SetBackgroundColor(color);*/

	m_UIWindows[0] = MakeUnique<UIWindow>(River::Move(panel));
}

void RiverMainUiLayer::OnDetach()
{

}

void RiverMainUiLayer::OnUpdate(float deltaTime)
{
	for (auto& it : m_UIWindows)
	{
		it.second->OnUpdate(deltaTime);
	}
}

bool RiverMainUiLayer::OnEvent(const Event& e)
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

void RiverMainUiLayer::OnRender()
{
	auto uiRenderPass = Application::Get()->GetRenderScene()->GetUIRenderPass();
	auto& renderVertices = uiRenderPass->GetRenderVertices();
	auto& renderIndices = uiRenderPass->GetRenderIndices();

	for (auto& window : m_UIWindows)
	{
		window.second->OnRender(renderVertices, renderIndices);
	}
}
