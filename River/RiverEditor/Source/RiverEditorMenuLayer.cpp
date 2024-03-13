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
	/*auto list = DynamicCast<ListWidget>(panel->GetChildWidgetByName("MainList"));
	list->SetChildWidgetType("Text");
	list->SetListData<String>({ String("abcdcecefqafe"), String("abcdacece 商"), String("abcdcacec 周") });*/

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
