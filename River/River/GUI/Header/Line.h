#pragma once
#include "RiverHead.h"
#include "Math/Header/BaseStruct.h"
#include "Renderer/Mesh/Header/Mesh.h"

class Line
{
public:
	Line();

	~Line();

	void AddPoint(Float2 point) { m_Points.push_back(point); }

	void DrawLine(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices, Byte4 color);

private:
	V_Array<Float2> m_Points;
};