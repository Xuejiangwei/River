#include "RiverPch.h"
#include "RiverEditorMenuLayer.h"
#include "Application.h"
#include "Renderer/Header/RHI.h"
#include "Renderer/Header/AssetManager.h"
#include "Renderer/Header/RenderScene.h"
#include "Renderer/Pass/Header/RenderPassUI.h"
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
	auto pass = Application::Get()->GetRenderScene()->GetRenderPassByType(RenderPassType::UI);
	if (pass)
	{
		auto uiRenderPass = dynamic_cast<RenderPassUI*>(pass);
		auto& renderVertices = uiRenderPass->GetRenderVertices();
		auto& renderIndices = uiRenderPass->GetRenderIndices();

		for (auto& window : m_UIWindows)
		{
			window.second->OnRender(renderVertices, renderIndices);
		}
	}
}
