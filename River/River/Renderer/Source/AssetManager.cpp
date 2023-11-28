#include "RiverPch.h"
#include "Renderer/Header/AssetManager.h"
#include "Renderer/Mesh/Header/Mesh.h"

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

void MeshAssetManager::AddProduceMeshAsset(const char* path, Unique<Mesh>& mesh)
{
	m_CacheMeshes[path] = std::move(mesh);
}

MeshAssetManager& MeshAssetManager::GetAssetManager()
{
	return *s_AssetManager.get();
}
