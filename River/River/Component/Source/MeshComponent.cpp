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
	/*if (m_StaticMesh)
	{
		RHI::Get()->UpdateSceneData(m_StaticMesh->GetVertices(), m_StaticMesh->GetIndices());
	}*/
}

void MeshComponent::SetStaticMesh(StaticMesh* staticMesh)
{
	m_StaticMesh = staticMesh;
}