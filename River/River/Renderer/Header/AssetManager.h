#pragma once
#include "RiverHead.h"
#include "Renderer/Header/Texture.h"
#include "Renderer/Header/Shader.h"

class Mesh;
class StaticMesh;

class AssetManager
{
	friend Texture* Texture::CreateTexture(const char* name, const char* filePath, bool isImmediately);
	friend Texture* Texture::CreateTexture(const char* name, int width, int height, const uint8* data);
	friend Texture* Texture::CreateTextureWithResource(const char* name, void* resoure);
public:
	AssetManager();
	
	~AssetManager();

	void LoadAsset(const char* path);

	Mesh* AddProduceMeshAsset(Unique<Mesh>& mesh);

	StaticMesh* AddStaticMesh(Unique<StaticMesh>& mesh);

	StaticMesh* GetStaticMesh(const char* name);

	//Texture
	Texture* GetTexture(const char* name);

	Texture* GetOrCreateTexture(const char* name, const char* path);

	//Shader
	Shader* GetShader(const char* name);

private:
	void AddCacheTexture(const char* name, Unique<Texture>& texture);

public:
	static AssetManager* Get();

private:
	static Unique<AssetManager> s_AssetManager;

	HashMap<String, Unique<Mesh>> m_CacheMeshes;
	HashMap<String, Unique<StaticMesh>> m_CacheStaticMeshes;
	HashMap<String, Unique<Texture>> m_CacheTextures;
	HashMap<String, Unique<Shader>> m_CacheShaders;
};