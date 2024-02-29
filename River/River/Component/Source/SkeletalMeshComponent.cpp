#include "RiverPch.h"
#include "Component/Header/SkeletalMeshComponent.h"
#include "Renderer/Mesh/Header/SkeletalMesh.h"

SkeletalMeshComponent::SkeletalMeshComponent()
	: m_SkeletalMesh(nullptr)
{
}

SkeletalMeshComponent::~SkeletalMeshComponent()
{
}

void SkeletalMeshComponent::Tick(float deltaTime)
{
}

void SkeletalMeshComponent::SetSkeletalMesh(SkeletalMesh* skeletalMesh)
{
	m_SkeletalMesh = skeletalMesh;
}

MaterialBlendMode SkeletalMeshComponent::GetMaterialMode() const
{
	return m_Materials.size() > 0 ? m_Materials[0]->m_BlendMode : MaterialBlendMode::Opaque;
}
