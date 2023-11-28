#include "RiverPch.h"
#include "Renderer/Mesh/Header/StaticMesh.h"
#include "Renderer/Header/RawPolyhedron.h"
#include "Renderer/Header/RHI.h"

StaticMesh::StaticMesh(V_Array<Vertex>& vertices, V_Array<uint32>& indices, V_Array<class Material*>& materials)
	: Mesh(vertices, indices, materials)
{
	RHI::Get()->SetUpStaticMesh(m_Vertices, m_Indices);
}

StaticMesh::~StaticMesh()
{
}