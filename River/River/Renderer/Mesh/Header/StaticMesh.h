#pragma once
#include "RiverHead.h"
#include "Mesh.h"

class StaticMesh
{
public:
	StaticMesh();

	~StaticMesh();

	const V_Array<Vertex>& GetVertices() const { return m_Vertices; }

	const V_Array<uint16>& GetIndices() const { return m_Indices; }

private:
	V_Array<Vertex> m_Vertices;
	V_Array<uint16> m_Indices;
};