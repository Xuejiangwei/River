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

MeshAssetManager& MeshAssetManager::GetAssetManager()
{
	return *s_AssetManager.get();
}
