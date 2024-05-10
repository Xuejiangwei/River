#pragma once

#include "RiverHead.h"
#include "Renderer/MeshAnimation/Header/MeshAnimationData.h"

struct UIVertex
{
	UIVertex() = default;
	
	UIVertex(float x, float y, float z, float u, float v)
		: Pos(x, y, z), TexC(u, v) 
	{
		Color[0] = Color[1] = Color[2] = Color[3] = 125;
	}

	UIVertex(float x, float y, float z, float u, float v, uint8 r, uint8 g, uint8 b, uint8 a)
		: Pos(x, y, z), TexC(u, v)
	{
		Color[0] = r;
		Color[1] = g;
		Color[2] = b;
		Color[3] = a;
	}

	Float3 Pos;
	Float2 TexC;
	uint8 Color[4];
};

struct RawVertex
{
	RawVertex() = default;

	RawVertex(float x, float y, float z, float u, float v, uint8 r, uint8 g, uint8 b, uint8 a)
		: Pos(x, y, z), TexC(u, v) 
	{
		Color[0] = r;
		Color[1] = g;
		Color[2] = b;
		Color[3] = a;
	}

	Float3 Pos;
	uint8 Color[4];
	Float2 TexC;
};

struct Vertex
{
	Vertex() = default;

	Vertex(float x, float y, float z, float nx, float ny, float nz, float tx, float ty, float tz, float u, float v)
		: Pos(x, y, z), Normal(nx, ny, nz), TangentU(tx, ty, tz), TexC(u, v)
	{}

	Vertex(Float3 pos, Float3 normal, Float3 tangent, Float2 tex)
		: Pos(pos), Normal(normal), TangentU(tangent), TexC(tex)
	{}

	Float3 Pos;
	Float3 Normal;
	Float2 TexC;
	Float3 TangentU;
};

struct SkeletalVertex
{
	SkeletalVertex() = default;

	SkeletalVertex(Float3 pos, Float3 normal, Float3 tangent, Float2 tex, Float3 weights, uint8 boneIndices[4])
		: Pos(pos), Normal(normal), TangentU(tangent), TexC(tex), BoneWeights(weights)
	{
		memcpy(BoneIndices, boneIndices, sizeof(BoneIndices));
	}

	Float3 Pos;
	Float3 Normal;
	Float2 TexC;
	Float3 TangentU;
	Float3 BoneWeights;
	uint8 BoneIndices[4];
};

struct StaticMeshData
{
	V_Array<Vertex> Vertices;
	V_Array<uint32> Indices;
	V_Array<class Material*> Materials;
};

struct SkeletalSubset
{
	uint32 Id = -1;
	uint32 VertexStart = 0;
	uint32 VertexCount = 0;
	uint32 IndexStart = 0;
	uint32 IndexCount = 0;
};

struct SkeletalMeshData
{
	V_Array<SkeletalVertex> Vertices;
	V_Array<uint32> Indices;
	V_Array<SkeletalSubset> Subsets;
	V_Array<class Material*> Materials;

	std::vector<int> BoneHierarchy;
	std::vector<Matrix4x4> BoneOffsets;
	HashMap<String, AnimationClip> AnimClips;

	/* 暂时用作fbx格式的cluster的逆矩阵，绑定骨骼点时冻结时cluster的位置，
	   而骨骼的位置是相对cluster空间的，所以offset矩阵是变换到cluster空间的骨骼空间，
	   最后还需要从cluster逆矩阵转换到世界原点空间*/
	std::vector<Matrix4x4> BoneOffsets1; 
};


class Mesh
{
public:
	Mesh(String& name, String& path);

	virtual ~Mesh();

	const String& GetName() const { return m_Name; }

	const V_Array<Vertex>& GetVertices() const { return m_StaticMeshData->Vertices; }

	const V_Array<SkeletalVertex>& GetSkeletalVertices() const { return m_SkeletalMeshData->Vertices; }

	const V_Array<uint32>& GetIndices() const { return m_StaticMeshData->Indices; }

	const V_Array<uint32>& GetSkeletalIndices() const { return m_SkeletalMeshData->Indices; }

	const Unique<SkeletalMeshData>& GetSkeletalMeshData() const { return m_SkeletalMeshData; }

protected:
	String m_Name;
	String m_Path;
	
	union
	{
		Unique<StaticMeshData> m_StaticMeshData;
		Unique<SkeletalMeshData> m_SkeletalMeshData;
	};
};
