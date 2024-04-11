#include "RiverPch.h"
#include "Renderer/Mesh/Header/SkeletalMesh.h"
#include "Renderer/Mesh/Header/MeshUtility.h"
#include "Renderer/Header/RHI.h"

SkeletalMesh::SkeletalMesh(String&& name, String&& path)
	: Mesh(River::Move(name), River::Move(path))
{
	m_SkeletalMeshData = MakeUnique<SkeletalMeshData>();
	LoadSkeletalMesh(m_Path, m_SkeletalMeshData.get());
	RHI::Get()->SetUpSkeletalMesh(this);
}

SkeletalMesh::~SkeletalMesh()
{
}
