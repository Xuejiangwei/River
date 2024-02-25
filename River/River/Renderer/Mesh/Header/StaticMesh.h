#pragma once
#include "RiverHead.h"
#include "Mesh.h"

class StaticMesh : public Mesh
{
public:
	StaticMesh(String& name, String& path);

	StaticMesh(const char* name, V_Array<Vertex>& vertices, V_Array<uint32>& indices, V_Array<class Material*>& materials);

	virtual ~StaticMesh() override;
};