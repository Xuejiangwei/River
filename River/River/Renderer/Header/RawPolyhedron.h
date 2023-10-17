#pragma once
#include "RiverHead.h"
#include "Renderer/Mesh/Header/Mesh.h"

struct RawPolyhedronData
{
	V_Array<RawVertex> Vertices;
	V_Array<uint16> Indices;
};

class RawPolyhedron
{
public:
	static RawPolyhedronData GetRawBox_8_Points();

	static RawPolyhedronData GetRawBox_24_Points();

	static RawPolyhedronData GetRawPlane();

	static RawPolyhedronData GetRawSphere();
};
