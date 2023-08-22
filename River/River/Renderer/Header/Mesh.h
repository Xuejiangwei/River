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

	River::Float3 Pos;
	River::Float2 TexC;
	uint8_t Color[4];
};

struct Vertex
{
	Vertex() = default;
	Vertex(float x, float y, float z, float nx, float ny, float nz, float u, float v) :
		Pos(x, y, z),
		Normal(nx, ny, nz),
		TexC(u, v) {}

	River::Float3 Pos;
	River::Float3 Normal;
	River::Float2 TexC;
};

class Mesh
{
public:
	Mesh(V_Array<Vertex>& vertices, V_Array<uint32_t>& indices);

	~Mesh();

private:
	V_Array<Vertex> m_Vertices;
	V_Array<uint32_t> m_Indices;
};
