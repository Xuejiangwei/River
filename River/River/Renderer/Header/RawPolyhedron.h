#pragma once
#include "RiverHead.h"
#include "Mesh.h"

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
