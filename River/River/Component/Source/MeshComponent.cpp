#include "RiverPch.h"
#include "Renderer/Header/RHI.h"
#include "Renderer/Mesh/Header/StaticMesh.h"
#include "Component/Header/MeshComponent.h"

StaticMeshComponent::StaticMeshComponent()
	: m_StaticMesh(nullptr)
{
}

StaticMeshComponent::~StaticMeshComponent()
{
}

void StaticMeshComponent::Tick(float deltaTime)
{
}

void StaticMeshComponent::SetStaticMesh(StaticMesh* staticMesh)
{
	m_StaticMesh = staticMesh;
}

MaterialBlendMode StaticMeshComponent::GetMaterialMode() const
{
	return m_Materials.size() > 0 ? m_Materials[0]->m_BlendMode : MaterialBlendMode::Opaque;
}
