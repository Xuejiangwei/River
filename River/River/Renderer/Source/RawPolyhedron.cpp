#include "RiverPch.h"
#include "Renderer/Header/RawPolyhedron.h"

RawPolyhedronData RawPolyhedron::GetRawBox_8_Points()
{
	RawPolyhedronData data;
	data.Vertices.resize(8);
	data.Indices.resize(36);

	data.Vertices[0] = { +0.5f, +0.5f, -0.5f, 0.0f, 0.0f, 255, 0, 0, 255 };
	data.Vertices[1] = { +0.5f, +0.5f, +0.5f, 1.0f, 0.0f, 255, 0, 0, 255 };
	data.Vertices[2] = { +0.5f, -0.5f, +0.5f, 0.0f, 1.0f, 255, 0, 0, 255 };
	data.Vertices[3] = { +0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 255, 0, 0, 255 };

	data.Vertices[4] = { -0.5f, +0.5f, -0.5f, 0.0f, 0.0f, 255, 0, 0, 255 };
	data.Vertices[5] = { -0.5f, +0.5f, +0.5f, 1.0f, 0.0f, 255, 0, 0, 255 };
	data.Vertices[6] = { -0.5f, -0.5f, +0.5f, 0.0f, 1.0f, 255, 0, 0, 255 };
	data.Vertices[7] = { -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 255, 0, 0, 255 };


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

RawPolyhedronData RawPolyhedron::GetRawBox_24_Points()
{
	RawPolyhedronData data;
	data.Vertices.resize(24);
	data.Indices.resize(36);

	//右
	data.Vertices[0] = { +0.5f, +0.5f, -0.5f, 0.0f, 0.0f, 255, 0, 0, 255 };
	data.Vertices[1] = { +0.5f, +0.5f, +0.5f, 1.0f, 0.0f, 255, 0, 0, 255 };
	data.Vertices[2] = { +0.5f, -0.5f, +0.5f, 0.0f, 1.0f, 255, 0, 0, 255 };
	data.Vertices[3] = { +0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 255, 0, 0, 255 };

	data.Indices[0] = 0;
	data.Indices[1] = 1;
	data.Indices[2] = 2;

	data.Indices[3] = 0;
	data.Indices[4] = 2;
	data.Indices[5] = 3;
	//后
	data.Vertices[4] = { +0.5f, +0.5f, +0.5f, 0.0f, 0.0f, 255, 0, 0, 255 };
	data.Vertices[5] = { -0.5f, +0.5f, +0.5f, 1.0f, 0.0f, 255, 0, 0, 255 };
	data.Vertices[6] = { -0.5f, -0.5f, +0.5f, 0.0f, 1.0f, 255, 0, 0, 255 };
	data.Vertices[7] = { +0.5f, -0.5f, +0.5f, 1.0f, 1.0f, 255, 0, 0, 255 };

	data.Indices[6] = 4;
	data.Indices[7] = 5;
	data.Indices[8] = 6;

	data.Indices[9] = 4;
	data.Indices[10] = 6;
	data.Indices[11] = 7;

	//左
	data.Vertices[8] = { -0.5f, +0.5f, +0.5f, 0.0f, 0.0f, 255, 0, 0, 255 };
	data.Vertices[9] = { -0.5f, +0.5f, -0.5f, 1.0f, 0.0f, 255, 0, 0, 255 };
	data.Vertices[10] = { -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 255, 0, 0, 255 };
	data.Vertices[11] = { -0.5f, -0.5f, +0.5f, 1.0f, 1.0f, 255, 0, 0, 255 };

	data.Indices[12] = 8;
	data.Indices[13] = 9;
	data.Indices[14] = 10;

	data.Indices[15] = 8;
	data.Indices[16] = 10;
	data.Indices[17] = 11;

	//前
	data.Vertices[12] = { -0.5f, +0.5f, -0.5f, 0.0f, 0.0f, 255, 0, 0, 255 };
	data.Vertices[13] = { +0.5f, +0.5f, -0.5f, 1.0f, 0.0f, 255, 0, 0, 255 };
	data.Vertices[14] = { +0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 255, 0, 0, 255 };
	data.Vertices[15] = { -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 255, 0, 0, 255 };

	data.Indices[18] = 12;
	data.Indices[19] = 13;
	data.Indices[20] = 14;

	data.Indices[21] = 12;
	data.Indices[22] = 14;
	data.Indices[23] = 15;

	//上
	data.Vertices[16] = { -0.5f, +0.5f, +0.5f, 0.0f, 0.0f, 255, 0, 0, 255 };
	data.Vertices[17] = { +0.5f, +0.5f, +0.5f, 1.0f, 0.0f, 255, 0, 0, 255 };
	data.Vertices[18] = { +0.5f, +0.5f, -0.5f, 0.0f, 1.0f, 255, 0, 0, 255 };
	data.Vertices[19] = { -0.5f, +0.5f, -0.5f, 1.0f, 1.0f, 255, 0, 0, 255 };

	data.Indices[24] = 16;
	data.Indices[25] = 17;
	data.Indices[26] = 18;

	data.Indices[27] = 16;
	data.Indices[28] = 18;
	data.Indices[29] = 19;

	//下
	data.Vertices[20] = { -0.5f, -0.5f, +0.5f, 0.0f, 0.0f, 255, 0, 0, 255 };
	data.Vertices[21] = { +0.5f, -0.5f, +0.5f, 1.0f, 0.0f, 255, 0, 0, 255 };
	data.Vertices[22] = { +0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 255, 0, 0, 255 };
	data.Vertices[23] = { -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 255, 0, 0, 255 };

	data.Indices[30] = 20;
	data.Indices[31] = 22;
	data.Indices[32] = 21;

	data.Indices[33] = 20;
	data.Indices[34] = 23;
	data.Indices[35] = 22;

	return data;
}

RawPolyhedronData RawPolyhedron::GetRawPlane()
{
	RawPolyhedronData data;
	data.Vertices.resize(4);
	data.Indices.resize(6);

	data.Vertices[0] = { -0.5f, +0.5f, -0.5f, 0.0f, 0.0f, 255, 0, 0, 255 };
	data.Vertices[1] = { +0.5f, +0.5f, -0.5f, 1.0f, 0.0f, 255, 0, 0, 255 };
	data.Vertices[2] = { +0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 255, 0, 0, 255 };
	data.Vertices[3] = { -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 255, 0, 0, 255 };

	data.Indices[0] = 0;
	data.Indices[1] = 1;
	data.Indices[2] = 2;

	data.Indices[3] = 0;
	data.Indices[4] = 2;
	data.Indices[5] = 3;

	return data;
}

RawPolyhedronData RawPolyhedron::GetRawSphere()
{
	return RawPolyhedronData();
}
