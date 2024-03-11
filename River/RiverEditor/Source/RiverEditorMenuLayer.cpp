#include "RiverPch.h"
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
	RHI::Get()->SetShowUIDebugOutline(false);
	m_UIWindows[0] = MakeUnique<UIWindow>(DynamicCast<Panel>(
		DecodeGUI_File("F:\\GitHub\\River\\River\\UI\\MainUI.json")));
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
