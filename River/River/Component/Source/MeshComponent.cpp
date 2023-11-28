#include "RiverPch.h"
#include "Component/Header/MeshComponent.h"
#include "Renderer/Header/RHI.h"
#include "Renderer/Mesh/Header/StaticMesh.h"

MeshComponent::MeshComponent()
{
}

MeshComponent::~MeshComponent()
{
}

void MeshComponent::Tick(float deltaTime)
{
}

void MeshComponent::SetStaticMesh(StaticMesh* staticMesh)
{
	m_StaticMesh = staticMesh;
}

void MeshComponent::SetStaticMeshMaterials(V_Array<Material*> materials)
{
	m_StaticMesh->SetMeshMaterials(materials);
}