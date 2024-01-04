#pragma once

#include "RiverHead.h"

#include "Renderer/Font/Header/TTF_Info.h"

class Font
{
public:
	Font(const char* path, class FontAtlas* atlas, float pixelSize = 20.0f);

	~Font();

	float GetPixelSize() const { return m_SizePixels; }

	void BuildLookupTable();

	void AddGlyph(uint16 codepoint, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, float advance_x);

	void GrowIndex(int new_size);

	const ImFontGlyph* FindGlyph(uint16 c) const;

	void SetGlyphVisible(uint16 c, bool visible);

	const ImFontGlyph* FindGlyphNoFallback(uint16 c) const;

	Float2 CalcTextSize(const String& text, float size);

	static int ImTextCharFromUtf8(unsigned int* out_char, const char* in_text, const char* in_text_end);

private:
	//int MetricsTotalSurface;
	float m_SizePixels;

	V_Array<uint16>           IndexLookup;        // 12-16 // out //            // Sparse. Index glyphs by Unicode code-point.
	V_Array<ImFontGlyph>       Glyphs;


	class FontAtlas* m_Atlas;

	float                       FallbackAdvanceX;
	V_Array<float>             IndexAdvanceX;
	uint8                        Used4kPagesMap[(0xFFFF + 1) / 4096 / 8];
	const ImFontGlyph* FallbackGlyph;
	uint16                     FallbackChar;
	uint16                     EllipsisChar;
	short                       EllipsisCharCount;
	float                       EllipsisWidth;
	float                       EllipsisCharStep;
};