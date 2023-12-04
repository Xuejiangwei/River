#include "RiverPch.h"
#include "Component/Header/MeshComponent.h"
#include "Renderer/Header/RHI.h"
#include "Renderer/Mesh/Header/StaticMesh.h"

StaticMeshComponent::StaticMeshComponent()
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

void StaticMeshComponent::SetStaticMeshMaterials(V_Array<Material*> materials)
{
	m_StaticMesh->SetMeshMaterials(materials);
}