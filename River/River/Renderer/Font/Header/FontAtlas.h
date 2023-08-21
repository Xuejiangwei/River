#pragma once

#include "RiverHead.h"
#include "MathStruct.h"
#include "Renderer/Font/Header/TTF_Info.h"

// Temporary data for one source font (multiple source fonts can be merged into one destination ImFont)
// (C++03 doesn't allow instancing ImVector<> with function-local types so we declare the type here.)

struct ImFontBuildDstData
{
	int                 SrcCount;           // Number of source fonts targeting this destination font.
	int                 GlyphsHighest;
	int                 GlyphsCount;
	ImBitVector        GlyphsSet;          // This is used to resolve collision when multiple sources are merged into a same destination font.
};

enum ImFontAtlasFlags_
{
	ImFontAtlasFlags_None = 0,
	ImFontAtlasFlags_NoPowerOfTwoHeight = 1 << 0,   // Don't round the height to next power of two
	ImFontAtlasFlags_NoMouseCursors = 1 << 1,   // Don't build software mouse cursors into the atlas (save a little texture memory)
	ImFontAtlasFlags_NoBakedLines = 1 << 2,   // Don't build thick line textures into the atlas (save a little texture memory, allow support for point/nearest filtering). The AntiAliasedLinesUseTex features uses them, otherwise they will be rendered using polygons (more expensive for CPU/GPU).
};

class FontAtlas
{
public:
	FontAtlas(const char* path, float pixelSize);

	~FontAtlas();

	void GetTextureDataRGBA32(uint8** outPixels);

private:
	void GetTextureDataAsAlpha8(uint8** outPixels);

	void Build();

	void BuildTrueType();

	int AddCustomRectRegular(int width, int height);

//private:
public:
	V_Array<uint8> m_Data;
	V_Array<uint8> m_PixelAlpha8;
	V_Array<UINT8_4> m_PixelRGBA32;
	V_Array<FontAtlasCustomRect> m_CustomRects;

	int m_TextureWidth;
	int m_TextureHeight;
	FLOAT_2 m_TextureUvScale;
	FLOAT_2 m_TextureUvWhitePixel;
	FLOAT_4 m_TexUvLines[63 + 1];

	int m_PackIdMouseCursors; // Custom texture rectangle ID for white pixel and mouse cursors
	int m_PackIdLines;        // Custom texture rectangle ID for baked anti-aliased lines

	TTF_HeadInfo m_FontInfo;
	class Font* m_Font;

	int m_Flags = 0;
};
