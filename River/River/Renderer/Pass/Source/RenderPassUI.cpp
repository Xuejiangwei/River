#include "RiverPch.h"
#include "Renderer/Pass/Header/RenderPassUI.h"
#include "Renderer/Header/RenderScene.h"
#include "Renderer/Header/RHI.h"
#include "Renderer/Pass/Header/RenderPassForwardRendering.h"
#include "Application.h"
#include "Layer.h"

RenderPassUI::RenderPassUI()
{
	m_CommandId = RHI::Get()->AllocDrawCommand();
}

RenderPassUI::~RenderPassUI()
{
}

void RenderPassUI::Render()
{
	auto& rhi = RHI::Get();
	rhi->ClearUIRenderItem();
	auto renderScene = Application::Get()->GetRenderScene();

	auto [w, h] = Application::Get()->GetWindow()->GetWindowSize();
	rhi->SetViewPort(w, h);


	m_RenderVertices.clear();
	m_RenderIndices.clear();

	for (auto& layer : m_Layers)
	{
		layer->OnRender();
	}

	m_PassUniform.RenderTargetSize.x = (float)w;
	m_PassUniform.RenderTargetSize.y = (float)h;
	rhi->UpdatePassUniform(2, &m_PassUniform);
	RHI::Get()->UpdateUIData(m_RenderVertices, m_RenderIndices);
	rhi->DrawRenderPass(this, FrameBufferType::UI);
}

void RenderPassUI::AddUILayer(Share<Layer>& layer)
{
	for (auto& lay : m_Layers)
	{
		if (lay == layer) 
		{
			return;
		}
	}

	m_Layers.push_back(layer);
}
