#pragma once
#include "RiverHead.h"
#include "Renderer/Header/Texture.h"
#include "Renderer/Header/Shader.h"
#include "Renderer/Header/Material.h"

class Mesh;
class StaticMesh;
class SkeletalMesh;

class AssetManager
{
	friend Texture* Texture::CreateTexture(const String& name, const String& filePath, bool isImmediately);
	friend Texture* Texture::CreateCubeTexture(const String& name, const String& filePath, bool isImmediately);
	friend Texture* Texture::CreateTexture(const String& name, int width, int height, const uint8* data);
	friend Texture* Texture::CreateTexture(const String& name, int width, int height);
	friend Shader* Shader::CreateShader(const String& name, const String& path, Pair<const ShaderDefine*, const ShaderDefine*> defines, ShaderParam* param);
	friend Material* Material::CreateMaterial(const String& name, const String& shaderName);
public:
	AssetManager();
	
	~AssetManager();

	void LoadAsset(const char* path);

	Mesh* AddProduceMeshAsset(Unique<Mesh>& mesh);

	StaticMesh* AddStaticMesh(Unique<StaticMesh>& mesh);

	StaticMesh* GetStaticMesh(const String& name);

	SkeletalMesh* AddSkeletalMesh(Unique<SkeletalMesh>& mesh);

	SkeletalMesh* GetSkeletalMesh(const char* name);

	//Texture
	Texture* GetTexture(const String& name);

	Texture* GetOrCreateTexture(const char* name, const char* path);

	//Shader
	Shader* GetShader(const String& name);

	//Material
	Material* GetMaterial(const String& name);

private:
	void AddCacheTexture(const String& name, Unique<Texture>& texture);

	void AddCacheShader(const String& name, Unique<Shader>& shader);

	void AddCacheMaterial(const char* name, Unique<Material>& material);

public:
	static AssetManager* Get();

private:
	static Unique<AssetManager> s_AssetManager;

	HashMap<String, Unique<Mesh>> m_CacheMeshes;
	HashMap<String, Unique<StaticMesh>> m_CacheStaticMeshes;
	HashMap<String, Unique<SkeletalMesh>> m_CacheSkeletalMeshes;
	HashMap<String, Unique<Texture>> m_CacheTextures;
	HashMap<String, Unique<Shader>> m_CacheShaders;
};