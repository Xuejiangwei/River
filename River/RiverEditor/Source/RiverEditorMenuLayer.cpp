﻿#include "RiverPch.h"
#include "RiverEditorMenuLayer.h"
#include "Application.h"
#include "Renderer/Header/RHI.h"
#include "Renderer/Header/AssetManager.h"
#include "Renderer/Header/RenderScene.h"
#include "Renderer/Pass/Header/RenderPassUI.h"
#include "GUI/Header/GuiDecode.h"
#include "GUI/Header/Panel.h"
#include "GUI/Header/Button.h"
#include "GUI/Header/Image.h"
#include "GUI/Header/Text.h"
#include "GUI/Header/Canvas.h"
#include "GUI/Header/ListWidget.h"
#include "GUI/Header/UIWindow.h"

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
	RHI::Get()->SetShowUIDebugOutline(true);
	auto panel = DynamicCast<Panel>(DecodeGUI_File("F:\\GitHub\\River\\River\\UI\\MainUI.json"));
	auto list = DynamicCast<ListWidget>(panel->GetChildWidgetByName("MainList"));
	list->SetChildWidgetType("Text");
	list->SetListData<String>({ String(u8"夏"), String(u8"商"), String(u8"周") });

	auto canvas = DynamicCast<Canvas>(panel->GetChildWidgetByName("MainCanvas"));
	uint8 color[4] = { 0, 255, 255, 255 };
	canvas->SetBackgroundColor(color);

	m_UIWindows[0] = MakeUnique<UIWindow>(River::Move(panel));
}

void RiverEditorMenuLayer::OnDetach()
{

}

void RiverEditorMenuLayer::OnUpdate(float deltaTime)
{
	for (auto& it : m_UIWindows)
	{
		it.second->OnUpdate(deltaTime);
	}
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
