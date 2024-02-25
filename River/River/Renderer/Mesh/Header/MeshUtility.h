#pragma once
#include "Mesh.h"

struct Subset
{
    uint32 Id = -1;
    uint32 VertexStart = 0;
    uint32 VertexCount = 0;
    uint32 FaceStart = 0;
    uint32 FaceCount = 0;
};

struct M3dMaterial
{
    std::string m_Name;

    Float4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
    Float3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
    float Roughness = 0.8f;
    bool AlphaClip = false;

    std::string MaterialTypeName;
    std::string DiffuseMapName;
    std::string NormalMapName;
};

bool LoadStaticMesh(const String& path, StaticMeshData* meshData);

bool LoadSkeletalMesh(const String& path, SkeletalMeshData* skeletalMeshData);