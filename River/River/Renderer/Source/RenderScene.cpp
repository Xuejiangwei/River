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
	RenderItem renderItem;

	//for (auto iter = m_Proxys.begin(); iter!= m_Proxys.end();)
	//{
	//	auto p = (RenderProxy*)*iter;
	//	if (p && p->m_RenderObject)
	//	{
	//		auto renderMeshComp = p->m_RenderObject->GetComponent<RenderMeshComponent>();
	//		if (renderMeshComp)
	//		{

	//			auto& compVertices = renderMeshComp->GetRawVertices();
	//			auto& compIndices = renderMeshComp->GetRawIndices();

	//			auto obj = p->m_RenderObject;
	//			renderItem.World = obj->GetTransform();
	//			renderItem.BaseVertexLocation = (int)vertices.size();
	//			renderItem.IndexCount = (int)compIndices.size();
	//			renderItem.StartIndexLocation = (int)indices.size();
	//			RHI::Get()->AddRenderItem(&renderItem);

	//			/*for (size_t i = 0; i < compIndices.size(); i++)
	//			{
	//				indices.push_back(compIndices[i] + vertices.size());
	//			}*/

	//			vertices.insert(vertices.end(), compVertices.begin(), compVertices.end());
	//			indices.insert(indices.end(), compIndices.begin(), compIndices.end());

	//			iter++;
	//		}
	//		
	//		auto staticMeshComp = p->m_RenderObject->GetComponent<StaticMeshComponent>();
	//		if (staticMeshComp)
	//		{
	//			auto obj = p->m_RenderObject;
	//			renderItem.World = obj->GetTransform();
	//			renderItem.BaseVertexLocation = 0;
	//			renderItem.IndexCount = (int)staticMeshComp->GetStaticMesh()->GetIndices().size();
	//			renderItem.StartIndexLocation = 0;
	//			if (staticMeshComp->GetStaticMesh()->GetMeshMaterials().size() > 0)
	//			{
	//				renderItem.Material = staticMeshComp->GetStaticMesh()->GetMeshMaterials()[0];
	//			}

	//			auto buffer = RHI::Get()->GetStaticMeshBuffer(staticMeshComp->GetStaticMesh()->GetName().c_str());
	//			renderItem.VertexBuffer = buffer.first;
	//			renderItem.IndexBuffer = buffer.second;

	//			if (renderItem.Material->m_Name == "MySkyMat")
	//			{
	//				RHI::Get()->AddRenderItem(&renderItem, "sky");

	//			}
	//			else
	//			{
	//				RHI::Get()->AddRenderItem(&renderItem);
	//			}


	//			iter++;
	//		}
	//	}
	//	else
	//	{
	//		m_Proxys.erase(iter++);
	//	}
	//}

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
