#include "RiverPch.h"
#include "Renderer/Mesh/Header/StaticMesh.h"
#include "Renderer/Mesh/Header/MeshUtility.h"
#include "Renderer/Header/RawPolyhedron.h"
#include "Renderer/Header/RHI.h"

StaticMesh::StaticMesh(String& name, String& path)
	: Mesh(name, path)
{
	LoadStaticMesh(m_Path, m_StaticMeshData.get());
}

StaticMesh::StaticMesh(const char* name, V_Array<Vertex>& vertices, V_Array<uint32>& indices, V_Array<class Material*>& materials)
	: Mesh(String(name), String("None"))
{
	m_StaticMeshData = MakeUnique<StaticMeshData>();
	m_StaticMeshData->Vertices = River::Move(vertices);
	m_StaticMeshData->Indices = River::Move(indices);
	m_StaticMeshData->Materials = River::Move(materials);
	
	RHI::Get()->SetUpStaticMesh(this);
}

StaticMesh::~StaticMesh()
{
}