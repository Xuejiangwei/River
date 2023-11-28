#pragma once
#include "RiverHead.h"

class Mesh;

class MeshAssetManager
{
public:
	MeshAssetManager();
	
	~MeshAssetManager();

	void LoadAsset(const char* path);

	void AddProduceMeshAsset(const char* path, Unique<Mesh>& mesh);

public:
	static MeshAssetManager& GetAssetManager();

private:
	static Unique<MeshAssetManager> s_AssetManager;

	HashMap<String, Unique<Mesh>> m_CacheMeshes;
};