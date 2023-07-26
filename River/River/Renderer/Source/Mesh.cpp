#include "RiverPch.h"
#include "Mesh.h"

Mesh::Mesh(V_Array<MeshVertex>& vertices, V_Array<unsigned int>& indices)
	: m_Vertices(River::Move(vertices)), m_Indices(River::Move(indices))
{
}

Mesh::~Mesh()
{
}