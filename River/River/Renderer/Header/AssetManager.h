#pragma once
#include "RiverHead.h"
#include "Renderer/Header/Texture.h"
#include "Renderer/Header/Shader.h"
#include "Renderer/Header/Material.h"

class Mesh;
class StaticMesh;

class AssetManager
{
	friend Texture* Texture::CreateTexture(const char* name, const char* filePath, bool isImmediately);
	friend Texture* Texture::CreateCubeTexture(const char* name, const char* filePath, bool isImmediately);
	friend Texture* Texture::CreateTexture(const char* name, int width, int height, const uint8* data);
	friend Texture* Texture::CreateTexture(const char* name, int width, int height);
	friend Shader* Shader::CreateShader(const char* name, const char* path, ShaderParam* param);
	friend Material* Material::CreateMaterial(const char* name, const char* shaderName);
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

	//Material
	Material* GetMaterial(const char* name);

private:
	void AddCacheTexture(const char* name, Unique<Texture>& texture);

	void AddCacheShader(const char* name, Unique<Shader>& shader);

	void AddCacheMaterial(const char* name, Unique<Material>& material);

public:
	static AssetManager* Get();

private:
	static Unique<AssetManager> s_AssetManager;

	HashMap<String, Unique<Mesh>> m_CacheMeshes;
	HashMap<String, Unique<StaticMesh>> m_CacheStaticMeshes;
	HashMap<String, Unique<Texture>> m_CacheTextures;
	HashMap<String, Unique<Shader>> m_CacheShaders;
};