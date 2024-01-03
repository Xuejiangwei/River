#include "RiverPch.h"
#include "Renderer/Header/AssetManager.h"
#include "Renderer/Mesh/Header/StaticMesh.h"

Unique<AssetManager> AssetManager::s_AssetManager = MakeUnique<AssetManager>();

AssetManager::AssetManager()
{
}

AssetManager::~AssetManager()
{
}

void AssetManager::LoadAsset(const char* path)
{
}

Mesh* AssetManager::AddProduceMeshAsset(Unique<Mesh>& mesh)
{
	auto meshRaw = mesh.get();
	m_CacheMeshes[mesh->GetName()] = std::move(mesh);
	return meshRaw;
}

StaticMesh* AssetManager::AddStaticMesh(Unique<StaticMesh>& mesh)
{
	auto meshPtr = mesh.get();
	m_CacheStaticMeshes[mesh->GetName()] = std::move(mesh);
	return meshPtr;
}

StaticMesh* AssetManager::GetStaticMesh(const char* name)
{
	auto iter = m_CacheStaticMeshes.find(name);
	if (iter != m_CacheStaticMeshes.end())
	{
		return iter->second.get();
	}

	return nullptr;
}

Texture* AssetManager::GetTexture(const char* name)
{
	auto iter = m_CacheTextures.find(name);
	if (iter != m_CacheTextures.end())
	{
		return iter->second.get();
	}

	return nullptr;
}

Texture* AssetManager::GetOrCreateTexture(const char* name, const char* path)
{
	auto iter = m_CacheTextures.find(name);
	if (iter != m_CacheTextures.end())
	{
		return iter->second.get();
	}

	return Texture::CreateTexture(name, path);
}

Shader* AssetManager::GetShader(const char* name)
{
	auto iter = m_CacheShaders.find(name);
	if (iter != m_CacheShaders.end())
	{
		return iter->second.get();
	}

	return nullptr;
}

void AssetManager::AddCacheTexture(const char* name, Unique<Texture>& texture)
{
	m_CacheTextures[name] = River::Move(texture);
}

AssetManager* AssetManager::Get()
{
	return s_AssetManager.get();
}
