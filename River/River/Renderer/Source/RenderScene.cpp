#include "RiverPch.h"
#include "Renderer/Header/RenderScene.h"
#include "Renderer/Header/RenderProxy.h"
#include "Renderer/Header/RHI.h"
#include "Object/Header/Object.h"
#include "Component/Header/RenderMeshComponent.h"
#include "Component/Header/MeshComponent.h"

#include "Renderer/Mesh/Header/StaticMesh.h"
extern Unique<StaticMesh> TestStaticMesh;

RenderScene::RenderScene()
{
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

	for (auto iter = m_Proxys.begin(); iter!= m_Proxys.end();)
	{
		auto p = (RenderProxy*)*iter;
		if (p && p->m_RenderObject)
		{
			auto renderMeshComp = p->m_RenderObject->GetComponent<RenderMeshComponent>();
			if (renderMeshComp)
			{

				auto& compVertices = renderMeshComp->GetRawVertices();
				auto& compIndices = renderMeshComp->GetRawIndices();

				auto obj = p->m_RenderObject;
				renderItem.World = obj->GetTransform();
				renderItem.BaseVertexLocation = (int)vertices.size();
				renderItem.IndexCount = (int)compIndices.size();
				renderItem.StartIndexLocation = (int)indices.size();
				RHI::Get()->AddRenderItem(&renderItem);

				/*for (size_t i = 0; i < compIndices.size(); i++)
				{
					indices.push_back(compIndices[i] + vertices.size());
				}*/

				vertices.insert(vertices.end(), compVertices.begin(), compVertices.end());
				indices.insert(indices.end(), compIndices.begin(), compIndices.end());

				iter++;
			}
			
			auto staticMeshComp = p->m_RenderObject->GetComponent<MeshComponent>();
			if (staticMeshComp)
			{
				auto obj = p->m_RenderObject;
				renderItem.World = obj->GetTransform();
				renderItem.BaseVertexLocation = 0;
				renderItem.IndexCount = (int)TestStaticMesh->GetIndices().size();
				renderItem.StartIndexLocation = 0;
				renderItem.ObjCBIndex = 1;
				renderItem.MaterialIndex = 10;

				RHI::Get()->AddRenderItem(&renderItem);

				iter++;
			}
		}
		else
		{
			m_Proxys.erase(iter++);
		}
	}

	RHI::Get()->UpdateSceneData(vertices, indices);
}

void RenderScene::AddObjectProxyToScene(RenderProxy* proxy)
{
	m_Proxys.insert(proxy);
}

void RenderScene::RemoveObjectProxyFromScene(RenderProxy* proxy)
{
	auto iter = m_Proxys.find(proxy);
	if (iter != m_Proxys.end())
	{
		m_Proxys.erase(iter);
	}
}
