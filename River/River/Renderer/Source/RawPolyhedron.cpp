#include "RiverPch.h"
#include "Renderer/Header/RawPolyhedron.h"

RawPolyhedronData RawPolyhedron::GetRawBox()
{
	RawPolyhedronData data;
	data.Vertices.resize(8);

	data.Vertices[0].Pos = { +0.5f, +0.5f, -0.5f };
	data.Vertices[1].Pos = { +0.5f, +0.5f, +0.5f };
	data.Vertices[2].Pos = { +0.5f, -0.5f, +0.5f };
	data.Vertices[3].Pos = { +0.5f, -0.5f, -0.5f };

	data.Vertices[4].Pos = { -0.5f, +0.5f, -0.5f };
	data.Vertices[5].Pos = { -0.5f, +0.5f, +0.5f };
	data.Vertices[6].Pos = { -0.5f, -0.5f, +0.5f };
	data.Vertices[7].Pos = { -0.5f, -0.5f, -0.5f };

	data.Indices.resize(36);

	data.Indices[0] = 0;
	data.Indices[1] = 1;
	data.Indices[2] = 2;

	data.Indices[3] = 0;
	data.Indices[4] = 2;
	data.Indices[5] = 3;

	data.Indices[6] = 1;
	data.Indices[7] = 5;
	data.Indices[8] = 6;

	data.Indices[9] = 1;
	data.Indices[10] = 6;
	data.Indices[11] = 2;

	data.Indices[12] = 5;
	data.Indices[13] = 4;
	data.Indices[14] = 7;

	data.Indices[15] = 5;
	data.Indices[16] = 7;
	data.Indices[17] = 6;

	data.Indices[18] = 4;
	data.Indices[19] = 0;
	data.Indices[20] = 3;

	data.Indices[21] = 4;
	data.Indices[22] = 3;
	data.Indices[23] = 7;

	data.Indices[24] = 5;
	data.Indices[25] = 1;
	data.Indices[26] = 0;

	data.Indices[27] = 5;
	data.Indices[28] = 0;
	data.Indices[29] = 4;

	data.Indices[30] = 6;
	data.Indices[31] = 2;
	data.Indices[32] = 3;

	data.Indices[33] = 6;
	data.Indices[34] = 3;
	data.Indices[35] = 7;

	return data;
}
