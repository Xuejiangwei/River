#pragma once

#include "RiverHead.h"


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

class StaticMesh;

class Mesh
{
public:
	Mesh(const char* name, V_Array<Vertex>& vertices, V_Array<uint32>& indices, V_Array<class Material*>& materials);

	~Mesh();

	const String& GetName() const { return m_Name; }

	const V_Array<Vertex>& GetVertices() const { return m_Vertices; }

	const V_Array<uint32>& GetIndices() const { return m_Indices; }

protected:
	String m_Name;
	V_Array<Vertex> m_Vertices;
	V_Array<uint32> m_Indices;
};
