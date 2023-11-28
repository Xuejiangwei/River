#pragma once
#include "RiverHead.h"
#include "Mesh.h"

class StaticMesh : public Mesh
{
public:
	StaticMesh(V_Array<Vertex>& vertices, V_Array<uint32>& indices, V_Array<class Material*>& materials);

	~StaticMesh();
};