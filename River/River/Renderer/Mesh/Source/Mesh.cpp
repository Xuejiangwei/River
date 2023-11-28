#include "RiverPch.h"
#include "Renderer/Mesh/Header/StaticMesh.h"
#include "Renderer/Header/Material.h"

Mesh::Mesh(V_Array<Vertex>& vertices, V_Array<uint32>& indices, V_Array<class Material*>& materials)
	: m_Vertices(River::Move(vertices)), m_Indices(River::Move(indices)), m_Materials(River::Move(materials))
{
}

Mesh::~Mesh()
{
}