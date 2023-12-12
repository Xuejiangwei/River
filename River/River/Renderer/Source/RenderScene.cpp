#include "RiverPch.h"
#include "Renderer/Header/RenderScene.h"
#include "Renderer/Header/RenderProxy.h"
#include "Renderer/Header/RHI.h"

#include "Renderer/Header/Material.h"
#include "Renderer/Pass/Header/RenderPassForwardRendering.h"

RenderScene::RenderScene()
{
	m_RenderPasses.push_back(MakeShare<RenderPassForwardRendering>());
}

RenderScene::~RenderScene()
{
}

void RenderScene::OnUpdate()
{
	RHI::Get()->ClearRenderItem();

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

	RHI::Get()->UpdateSceneData(vertices, indices);

	for (auto pass : m_RenderPasses)
	{
		pass->Render();
	}
}

void RenderScene::AddObjectProxyToScene(RenderProxy* proxy)
{
	if (proxy->HasRenderData())
	{
		m_Proxys[proxy->GetRenderBlendMode()].insert(proxy);
	}
}

void RenderScene::RemoveObjectProxyFromScene(RenderProxy* proxy)
{
	if (proxy->HasRenderData())
	{
		auto proxys = m_Proxys[proxy->GetRenderBlendMode()];
		auto iter = proxys.find(proxy);
		if (iter != proxys.end())
		{
			proxys.erase(iter);
		}
	}
	
}
