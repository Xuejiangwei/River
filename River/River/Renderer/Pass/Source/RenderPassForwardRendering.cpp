#include "RiverPch.h"
#include "Renderer/Pass/Header/RenderPassForwardRendering.h"
#include "Renderer/Header/RHI.h"
#include "Renderer/Header/RenderScene.h"
#include "Renderer/Header/RenderProxy.h"
#include "Application.h"

RenderPassForwardRendering::RenderPassForwardRendering()
{
	m_CommandId = RHI::Get()->AllocDrawCommand();
}

RenderPassForwardRendering::~RenderPassForwardRendering()
{
}

void RenderPassForwardRendering::Render()
{
	auto& rhi = RHI::Get();
	auto renderScene = Application::Get()->GetRenderScene();

	rhi->SetViewPort(720, 720);

	//先渲染非透明队列
	auto& renderProxys = renderScene->GetRenderProxys(MaterialBlendMode::Opaque);
	for (auto& proxy : renderProxys)
	{
		auto renderProxy = static_cast<RenderProxy*>(proxy);
		if (renderProxy)
		{	
			RenderItem renderItem;
			renderProxy->GetRenderData(renderItem);
			RHI::Get()->AddRenderItem(&renderItem);
		}
	}

	rhi->GenerateDrawCommands(m_CommandId);
	//rhi->Render();
}
