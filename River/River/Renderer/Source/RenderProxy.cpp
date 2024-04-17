#include "RiverPch.h"
#include "Renderer/Header/RHI.h"
#include "Renderer/Header/RenderProxy.h"
#include "Renderer/Header/RenderScene.h"
#include "Renderer/Header/RenderItem.h"
#include "Renderer/Mesh/Header/StaticMesh.h"
#include "Renderer/Mesh/Header/SkeletalMesh.h"
#include "Renderer/Header/Material.h"
#include "Object/Header/Object.h"
#include "Component/Header/RenderMeshComponent.h"
#include "Component/Header/StaticMeshComponent.h"
#include "Component/Header/SkeletalMeshComponent.h"
#include "Application.h"
#include "Math/Header/Geometric.h"

RenderProxy::RenderProxy(Object* object)
	: m_RenderObject(object), m_IsDirty(true)
{
	m_RenderItemIds.clear();
	m_ProxyId = Application::Get()->GetRenderScene()->AddObjectProxyToScene(this);
}

RenderProxy::~RenderProxy()
{
	Application::Get()->GetRenderScene()->RemoveObjectProxyFromScene(this);

	for (auto& id : m_RenderItemIds)
	{
		RHI::Get()->RemoveRenderItem(id);
	}

	m_RenderItemIds.clear();
}

void RenderProxy::GetRenderData()
{
	m_IsDirty = false;
	bool hasRenderItem = HasRenderItem();
	auto staticMeshComp = m_RenderObject->GetComponent<StaticMeshComponent>();
	if (staticMeshComp)
	{
		auto renderItem = hasRenderItem ? RHI::Get()->GetRenderItem(m_RenderItemIds[0]) : RHI::Get()->AddRenderItem();

		renderItem->NumFramesDirty = RHI::GetFrameCount();
		renderItem->World = m_RenderObject->GetTransform();
		renderItem->BaseVertexLocation = 0;
		renderItem->IndexCount = (int)staticMeshComp->GetStaticMesh()->GetIndices().size();
		renderItem->StartIndexLocation = 0;
		if (staticMeshComp->GetMeshMaterials().size() > 0)
		{
			renderItem->Material = staticMeshComp->GetMeshMaterials()[0];
		}

		/*if (renderItem.Material->m_Name == "MySkyMat")
		{
			RHI::Get()->AddRenderItem(&renderItem, "sky");

		}
		else
		{
			RHI::Get()->AddRenderItem(&renderItem);
		}*/

		auto buffer = RHI::Get()->GetStaticMeshBuffer(staticMeshComp->GetStaticMesh()->GetName().c_str());
		renderItem->VertexBuffer = buffer.first;
		renderItem->IndexBuffer = buffer.second;
		renderItem->TexTransform = Matrix4x4_Scaling(8, 8, 1);

		if (!hasRenderItem)
		{
			m_RenderItemIds.push_back(renderItem->ObjCBIndex);
		}
	}
	else
	{
		auto skmComp = m_RenderObject->GetComponent<SkeletalMeshComponent>();
		if (skmComp)
		{
			auto skeletalMesh = skmComp->GetSkeletalMesh();
			for (auto i = 0; i < skeletalMesh->GetSkeletalMeshData()->Subsets.size(); i++)
			{
				auto renderItem = hasRenderItem ? RHI::Get()->GetRenderItem(m_RenderItemIds[i]) : RHI::Get()->AddRenderItem();
				auto subset = skeletalMesh->GetSkeletalMeshData()->Subsets[i];

				renderItem->NumFramesDirty = RHI::GetFrameCount();
				renderItem->World = m_RenderObject->GetTransform();
				renderItem->BaseVertexLocation = 0;
				renderItem->IndexCount = subset.IndexCount;
				renderItem->StartIndexLocation = subset.IndexStart;

				renderItem->AnimTransforms = skmComp->GetAnimFinalTransforms();
				if (skmComp->GetSkeletalMeshMaterials().size() > i)
				{
					renderItem->Material = skmComp->GetSkeletalMeshMaterials()[i];
				}

				auto buffer = RHI::Get()->GetStaticMeshBuffer(skmComp->GetSkeletalMesh()->GetName().c_str());
				renderItem->VertexBuffer = buffer.first;
				renderItem->IndexBuffer = buffer.second;

				if (!hasRenderItem)
				{
					m_RenderItemIds.push_back(renderItem->ObjCBIndex);
				}
			}
		}
	}
}

bool RenderProxy::HasRenderData() const
{
	return m_RenderObject->GetComponent<StaticMeshComponent>() || m_RenderObject->GetComponent<SkeletalMeshComponent>();
}

MaterialBlendMode RenderProxy::GetRenderBlendMode() const
{
	auto meshComp = m_RenderObject->GetComponent<StaticMeshComponent>();
	if (meshComp)
	{
		return meshComp->GetMaterialMode();
	}
	else
	{
		auto skmComp = m_RenderObject->GetComponent<SkeletalMeshComponent>();
		if (skmComp)
		{
			return skmComp->GetMaterialMode();
		}
	}

	return MaterialBlendMode::Opaque;
}

void RenderProxy::AddLightObjectProxy()
{
	m_ProxyId = Application::Get()->GetRenderScene()->AddLightObjectProxyToScene(this);
}

void RenderProxy::AddCameraObjectProxy()
{
	m_ProxyId = Application::Get()->GetRenderScene()->AddCameraObjectProxyToScene(this);
}

void RenderProxy::AddSkyBoxObjectProxy()
{
	m_ProxyId = Application::Get()->GetRenderScene()->AddSkyBoxObjectProxyToScene(this);
}
