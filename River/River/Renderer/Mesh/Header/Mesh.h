#pragma once

#include "RiverHead.h"
#include "MathStruct.h"

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

	FLOAT_3 Pos;
	FLOAT_2 TexC;
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

	FLOAT_3 Pos;
	uint8 Color[4];
	FLOAT_2 TexC;
};

struct Vertex
{
	Vertex() = default;

	Vertex(float x, float y, float z, float nx, float ny, float nz, float u, float v)
		: Pos(x, y, z), Normal(nx, ny, nz), TexC(u, v)
	{}

	FLOAT_3 Pos;
	FLOAT_3 Normal;
	FLOAT_3 TangentU;
	FLOAT_2 TexC;
};

class Mesh
{
public:
	Mesh(V_Array<Vertex>& vertices, V_Array<uint32>& indices);

	~Mesh();

private:
	V_Array<Vertex> m_Vertices;
	V_Array<uint32> m_Indices;
};
