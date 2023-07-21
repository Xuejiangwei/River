#pragma once

#include "RiverHead.h"
#include "Vector.h"

struct Vertex
{
	Vertex() {}
	Vertex(const Vector3& p, const Vector3& n, const Vector3& t, const Vector2& uv) 
		: Position(p), Normal(n), TangentU(t),TexC(uv)
	{}
	Vertex(float px, float py, float pz, float nx, float ny, float nz, float tx, float ty, float tz, float u, float v) 
		: Position(px, py, pz), Normal(nx, ny, nz), TangentU(tx, ty, tz), TexC(u, v) 
	{}

	Vector3 Position;
	Vector3 Normal;
	Vector3 TangentU;
	Vector2 TexC;
};

struct MeshData
{
	std::vector<Vertex> Vertices;
	std::vector<uint32_t> Indices32;
};

class GeometryGenerator
{
public:
	GeometryGenerator()
		: m_Subdivisions(true), m_SubdivisionsNum(3)
	{};
	
	~GeometryGenerator() {};

private:
	void Initialize();

	void Subdivide(MeshData& meshData);

	Vertex MidPoint(const Vertex& v0, const Vertex& v1);

	void CreatePlane();

	void CreateCube();

	void CreateSphere();
public:
	static Unique<GeometryGenerator>& Get();

private:
	static Unique<GeometryGenerator> s_Instance;

	MeshData m_MeshPlane;
	MeshData m_MeshCube;
	MeshData m_MeshSphere;

	bool m_Subdivisions;
	uint32_t m_SubdivisionsNum;
};