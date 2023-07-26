#pragma once

#include "RiverHead.h"
#include "MathStruct.h"

struct MeshVertex
{
	MeshVertex() = default;
	MeshVertex(float x, float y, float z, float nx, float ny, float nz, float u, float v) :
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
	Mesh(V_Array<MeshVertex>& vertices, V_Array<unsigned int>& indices);

	~Mesh();

private:
	V_Array<MeshVertex> m_Vertices;
	V_Array<unsigned int> m_Indices;
};
