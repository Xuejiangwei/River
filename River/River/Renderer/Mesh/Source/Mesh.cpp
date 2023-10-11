#include "RiverPch.h"
#include "Renderer/Mesh/Header/Mesh.h"

Mesh::Mesh(V_Array<Vertex>& vertices, V_Array<uint32_t>& indices)
	: m_Vertices(River::Move(vertices)), m_Indices(River::Move(indices))
{
}

Mesh::~Mesh()
{
}