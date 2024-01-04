#pragma once
#include "RiverHead.h"
#include "Renderer/Mesh/Header/StaticMesh.h"

class StaticMesh;

class GeometryGenerator
{
public:
	static StaticMesh* CreateBoxStaticMesh(float width, float height, float depth, uint32 numSubdivisions);

	static StaticMesh* CreateSphereStaticMesh(float radius, uint32_t sliceCount, uint32_t stackCount);

	static StaticMesh* CreateGridStaticMesh(float width, float depth, uint32_t m, uint32_t n);

private:
	static void Subdivide(V_Array<Vertex>& vertices, V_Array<uint32>& indices);

	static Vertex MidPoint(const Vertex& v0, const Vertex& v1);
};