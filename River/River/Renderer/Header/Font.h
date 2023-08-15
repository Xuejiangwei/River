#pragma once

#include "RiverHead.h"

struct TtfHead
{
	int loca, head, glyf, hhea, hmtx, kern, gpos, svg;
};

class Font
{
public:
	Font(const char* path);

	~Font();

	void LoadTtfFileData(const char* path);

	void LoadFntFileData(const char* path);

private:
	V_Array<unsigned char> m_Data;
	float m_SizePixels = 18.f;
	TtfHead m_HeadInfo;
};