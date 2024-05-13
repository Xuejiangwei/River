#pragma once

#include "RiverHead.h"

#include "Renderer/Font/Header/TTF_Info.h"

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
	FontAtlas(const String& path, float pixelSize);

	~FontAtlas();

	const uint8* GetTextureDataRGBA32();

	int GetTextureWidth() const { return m_TextureWidth; }

	int GetTextureHeight() const { return m_TextureHeight; }

private:
	const uint8* GetTextureDataAsAlpha8();

	void Build();

	void BuildTrueType();

	int AddCustomRectRegular(int width, int height);

//private:
public:
	V_Array<uint8> m_Data;
	V_Array<uint8> m_PixelAlpha8;
	V_Array<Byte4> m_PixelRGBA32;
	V_Array<FontAtlasCustomRect> m_CustomRects;

	int m_TextureWidth;
	int m_TextureHeight;
	Float2 m_TextureUvScale;
	Float2 m_TextureUvWhitePixel;
	Float4 m_TexUvLines[63 + 1];

	int m_PackIdMouseCursors; // Custom texture rectangle ID for white pixel and mouse cursors
	int m_PackIdLines;        // Custom texture rectangle ID for baked anti-aliased lines

	TTF_HeadInfo m_FontInfo;
	Unique<class Font> m_Font;

	int m_Flags = 0;
};
