#pragma once
#include "RiverHead.h"
#include "Math/Header/BaseStruct.h"

class Line
{
public:
	Line();

	~Line();

private:
	V_Array<Float2> m_Points;
};