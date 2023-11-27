#pragma once
#include "RiverHead.h"

class Mesh;

class MeshAssetManager
{
public:
	MeshAssetManager();
	
	~MeshAssetManager();

	static MeshAssetManager& GetAssetManager();

private:
	static Unique<MeshAssetManager> s_AssetManager;

	HashMap<String, Unique<Mesh>> m_CacheMeshes;
};