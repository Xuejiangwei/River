#pragma once
#include "RiverHead.h"

class Mesh;
class StaticMesh;

class MeshAssetManager
{
public:
	MeshAssetManager();
	
	~MeshAssetManager();

	void LoadAsset(const char* path);

	Mesh* AddProduceMeshAsset(Unique<Mesh>& mesh);

	StaticMesh* AddStaticMesh(Unique<StaticMesh>& mesh);

	StaticMesh* GetStaticMesh(const char* name);

public:
	static MeshAssetManager& Get();

private:
	static Unique<MeshAssetManager> s_AssetManager;

	HashMap<String, Unique<Mesh>> m_CacheMeshes;
	HashMap<String, Unique<StaticMesh>> m_CacheStaticMeshes;
};