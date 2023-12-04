#include "RiverPch.h"
#include "Renderer/Mesh/Header/StaticMesh.h"
#include "Renderer/Header/RawPolyhedron.h"
#include "Renderer/Header/RHI.h"

StaticMesh::StaticMesh(const char* name, V_Array<Vertex>& vertices, V_Array<uint32>& indices, V_Array<class Material*>& materials)
	: Mesh(name, vertices, indices, materials)
{
	RHI::Get()->SetUpStaticMesh(this);
}

StaticMesh::~StaticMesh()
{
}