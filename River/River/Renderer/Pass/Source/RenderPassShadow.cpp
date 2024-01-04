#include "RiverPch.h"
#include "Renderer/Pass/Header/RenderPassShadow.h"
#include "Application.h"
#include "Renderer/Header/RHI.h"
#include "Renderer/Header/RenderScene.h"
#include "Renderer/Header/RenderProxy.h"
#include "Object/Header/LightObject.h"

RenderPassShadow::RenderPassShadow()
{
	m_CommandId = RHI::Get()->AllocDrawCommand();
}

RenderPassShadow::~RenderPassShadow()
{
}

void RenderPassShadow::Render()
{
	/*auto& rhi = RHI::Get();
	auto renderScene = Application::Get()->GetRenderScene();

	rhi->SetViewPort(720, 720);

	LightObject* mainLight = nullptr;
	auto& lightProxys = renderScene->GetRenderLightProxys();
	for (auto& proxy : lightProxys)
	{
		mainLight = dynamic_cast<LightObject*>(proxy->GetObject());
		if (mainLight)
		{
			break;
		}
	}


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

	rhi->GenerateDrawCommands(m_CommandId, FrameBufferType::ShadowMap);
	m_RenderBatch.clear();*/
}
