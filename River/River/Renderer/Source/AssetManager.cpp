#include "RiverPch.h"
#include "Renderer/Header/AssetManager.h"
#include "Renderer/Mesh/Header/StaticMesh.h"

Unique<MeshAssetManager> MeshAssetManager::s_AssetManager = MakeUnique<MeshAssetManager>();

MeshAssetManager::MeshAssetManager()
{
}

MeshAssetManager::~MeshAssetManager()
{
}

void MeshAssetManager::LoadAsset(const char* path)
{
}

Mesh* MeshAssetManager::AddProduceMeshAsset(Unique<Mesh>& mesh)
{
	auto meshRaw = mesh.get();
	m_CacheMeshes[mesh->GetName()] = std::move(mesh);
	return meshRaw;
}

StaticMesh* MeshAssetManager::AddStaticMesh(Unique<StaticMesh>& mesh)
{
	auto meshPtr = mesh.get();
	m_CacheStaticMeshes[mesh->GetName()] = std::move(mesh);
	return meshPtr;
}

StaticMesh* MeshAssetManager::GetStaticMesh(const char* name)
{
	auto iter = m_CacheStaticMeshes.find(name);
	if (iter != m_CacheStaticMeshes.end())
	{
		return iter->second.get();
	}

	return nullptr;
}

MeshAssetManager& MeshAssetManager::Get()
{
	return *s_AssetManager.get();
}
