#include "RiverPch.h"
#include "Renderer/Header/RHI.h"
#include "Renderer/Header/RenderProxy.h"
#include "Renderer/Header/RenderScene.h"
#include "Renderer/Header/RenderItem.h"
#include "Renderer/Mesh/Header/StaticMesh.h"
#include "Renderer/Header/Material.h"
#include "Object/Header/Object.h"
#include "Component/Header/RenderMeshComponent.h"
#include "Component/Header/MeshComponent.h"
#include "Application.h"

RenderProxy::RenderProxy(Object* object)
	: m_RenderObject(object), m_IsDirty(true), m_RenderItemId(-1)
{
	m_ProxyId = Application::Get()->GetRenderScene()->AddObjectProxyToScene(this);
}

RenderProxy::~RenderProxy()
{
	Application::Get()->GetRenderScene()->RemoveObjectProxyFromScene(this);
	RHI::Get()->RemoveRenderItem(m_RenderItemId);
}

void RenderProxy::GetRenderData(RenderItem* renderItem)
{
	m_IsDirty = false;
	auto staticMeshComp = m_RenderObject->GetComponent<StaticMeshComponent>();
	if (staticMeshComp)
	{
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
		m_RenderItemId = renderItem->ObjCBIndex;
	}
}

bool RenderProxy::HasRenderData() const
{
	return m_RenderObject->GetComponent<StaticMeshComponent>();
}

MaterialBlendMode RenderProxy::GetRenderBlendMode() const
{
	auto meshComp = m_RenderObject->GetComponent<StaticMeshComponent>();
	if (meshComp)
	{
		return meshComp->GetMaterialMode();
	}

	return MaterialBlendMode::Opaque;
}

void RenderProxy::AddLightObjectProxy()
{
	m_ProxyId = Application::Get()->GetRenderScene()->AddLightObjectProxyToScene(this);
}
