#pragma once
#include "RiverHead.h"
#include "Renderer/Mesh/Header/StaticMesh.h"

class StaticMesh;

class GeometryGenerator
{
public:
	static Unique<StaticMesh> CreateBoxStaticMesh(float width, float height, float depth, uint32 numSubdivisions);


private:
	static void Subdivide(V_Array<Vertex>& vertices, V_Array<uint32>& indices);

	static Vertex MidPoint(const Vertex& v0, const Vertex& v1);
};