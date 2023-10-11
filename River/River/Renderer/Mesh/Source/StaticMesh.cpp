#include "RiverPch.h"
#include "Renderer/Mesh/Header/StaticMesh.h"
#include "Renderer/Header/RawPolyhedron.h"

Unique<StaticMesh> StaticMesh::s_Box = nullptr;

StaticMesh::StaticMesh()
{
}

StaticMesh::~StaticMesh()
{
}

void StaticMesh::SetMeshData(const RawPolyhedronData& data)
{
	m_Vertices = data.Vertices;
	m_Indices = data.Indices;
}

StaticMesh* StaticMesh::GetBoxMesh()
{
	if (!s_Box)
	{
		s_Box = MakeUnique<StaticMesh>();
		s_Box->SetMeshData(RawPolyhedron::GetRawBox());
	}

	return s_Box.get();
}
