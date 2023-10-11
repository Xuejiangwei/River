#pragma once
#include "RiverHead.h"
#include "Renderer/Mesh/Header/Mesh.h"

struct RawPolyhedronData
{
	V_Array<Vertex> Vertices;
	V_Array<uint16> Indices;
};

class RawPolyhedron
{
public:
	static RawPolyhedronData GetRawBox();

};
