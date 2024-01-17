#include "RiverPch.h"
#include "Renderer/Pass/Header/RenderPassForwardRendering.h"
#include "Renderer/Header/RHI.h"
#include "Renderer/Header/RenderScene.h"
#include "Renderer/Header/RenderProxy.h"
#include "Object/Header/LightObject.h"
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

	//获得光源
	auto& lightProxys = renderScene->GetRenderLightProxys();
	for (size_t i = 0; i < lightProxys.size() && i < _countof(m_PassUniform.Lights); i++)
	{
		auto lightObject = dynamic_cast<LightObject*>(lightProxys[i]->GetObject());
		m_PassUniform.Lights[i].Direction = lightObject->GetDirection();
		m_PassUniform.Lights[i].Direction = lightObject->GetLightStrength();
	}
	
	rhi->UpdatePassUniform(0, &m_PassUniform);

	//先渲染非透明队列
	auto& renderProxys = renderScene->GetRenderProxys(MaterialBlendMode::Opaque);
	for (auto& proxy : renderProxys)
	{
		auto renderProxy = static_cast<RenderProxy*>(proxy);
		if (renderProxy)
		{	
			auto renderItem = rhi->GetRenderItem(renderProxy->GetRenderItemId());
			m_RenderBatch[renderItem->Material ? renderItem->Material->m_Shader->GetShaderId() : 0].push_back(renderProxy->GetRenderItemId());
		}
	}

	for (auto& batch : m_RenderBatch)
	{
		for (auto& renderItemId : batch.second)
		{
			rhi->DrawRenderItem(renderItemId);
		}
	}

	rhi->DrawRenderPass(this, FrameBufferType::Color);
	//rhi->Render();

	m_RenderBatch.clear();
}
