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
	: m_RenderObject(object)
{
	Application::Get()->GetRenderScene()->AddObjectProxyToScene(this);
}

RenderProxy::~RenderProxy()
{
	/*if (Application::Get().GetRenderScene())
	{
		Application::Get().GetRenderScene()->RemoveObjectProxyFromScene(this);
	}*/
}

void RenderProxy::GetRenderData(RenderItem& renderItem)
{
	auto staticMeshComp = m_RenderObject->GetComponent<StaticMeshComponent>();
	if (staticMeshComp)
	{
		renderItem.World = m_RenderObject->GetTransform();
		renderItem.BaseVertexLocation = 0;
		renderItem.IndexCount = (int)staticMeshComp->GetStaticMesh()->GetIndices().size();
		renderItem.StartIndexLocation = 0;
		if (staticMeshComp->GetStaticMesh()->GetMeshMaterials().size() > 0)
		{
			renderItem.Material = staticMeshComp->GetStaticMesh()->GetMeshMaterials()[0];
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
		renderItem.VertexBuffer = buffer.first;
		renderItem.IndexBuffer = buffer.second;
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
