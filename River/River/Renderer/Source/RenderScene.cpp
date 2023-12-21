#include "RiverPch.h"
#include "RiverTime.h"
#include "Renderer/Header/RenderScene.h"
#include "Renderer/Header/RenderProxy.h"
#include "Renderer/Header/RHI.h"

#include "Renderer/Header/Material.h"
#include "Renderer/Pass/Header/RenderPassForwardRendering.h"

RenderScene::RenderScene()
{
	m_UnuseProxyId.clear();
	m_Proxys.clear();
	m_RenderPasses.clear();

	m_RenderPasses.push_back(MakeShare<RenderPassForwardRendering>());
}

RenderScene::~RenderScene()
{
}

void RenderScene::Update(const RiverTime& time)
{
	for (auto& proxys : m_Proxys)
	{
		for (auto proxy : proxys.second)
		{
			auto p = (RenderProxy*)proxy;
			if (p->IsDirty())
			{
				if (p->HasRenderItem())
				{
					p->GetRenderData(RHI::Get()->GetRenderItem(p->GetRenderItemId()));
				}
				else
				{
					p->GetRenderData(RHI::Get()->AddRenderItem());
				}
			} 
		}
	}
}

void RenderScene::Render()
{
	V_Array<RawVertex> vertices;
	V_Array<uint16> indices;
	/*RenderItem renderItem;

	for (auto iter = m_Proxys[MaterialBlendMode::Opaque].begin(); iter != m_Proxys[MaterialBlendMode::Opaque].end();)
	{
		auto p = (RenderProxy*)*iter;
		if (p && p->IsValid())
		{
			if (!p->HasRenderData())
			{
				p->GetRenderData(RHI::Get()->AddRenderItem());
			}
		}
		else
		{
			m_Proxys[MaterialBlendMode::Opaque].erase(iter++);
		}
	}*/

	//RHI::Get()->UpdateSceneData(vertices, indices);

	RHI::Get()->BeginFrame();
	for (auto pass : m_RenderPasses)
	{
		pass->Render();
	}
	RHI::Get()->EndFrame();
}

int RenderScene::AddObjectProxyToScene(RenderProxy* proxy)
{
	int id = 0;
	if (proxy->HasRenderData())
	{
		if (m_UnuseProxyId.size() > 0)
		{
			id = m_UnuseProxyId.back();
			m_UnuseProxyId.resize(m_UnuseProxyId.size() - 1);
			m_Proxys[proxy->GetRenderBlendMode()][id] = proxy;
		}
		else
		{
			m_Proxys[proxy->GetRenderBlendMode()].push_back(proxy);
			id = (int)m_Proxys[proxy->GetRenderBlendMode()].size();
		}
	}

	return id;
}

void RenderScene::RemoveObjectProxyFromScene(RenderProxy* proxy)
{
	if (proxy->HasRenderData())
	{
		auto& proxys = m_Proxys[proxy->GetRenderBlendMode()];
		proxys[proxy->GetRenderProxyId()] = nullptr;
		m_UnuseProxyId.push_back(proxy->GetRenderProxyId());
	}
	
}
