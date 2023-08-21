#pragma once

#include "RiverHead.h"
#include "MathStruct.h"
#include "Renderer/Font/Header/TTF_Info.h"

class Font
{
public:
	Font(const char* path, class FontAtlas* atlas, float pixelSize = 20.0f);

	~Font();

	int GetTexHeight() const { return m_TexHeight; }
	void SetTexHeight(int h) { m_TexHeight = h; }

	int GetTexWidth() const { return m_TexWidth; }
	void SetTexWidth(int w) { m_TexWidth = w; }

	float GetPixelSize() const { return m_SizePixels; }

	V_Array<FontAtlasCustomRect>& GetRectsRef() { return m_CustomRects; }

	void SetTexUvScale(River::Float2 scale) { m_TexUvScale = scale; }

	uint8* GetTexPixelsAlpha8() { return const_cast<uint8*>(m_TexPixelsAlpha8.data()); }

	void BuildLookupTable();

	void AddGlyph(uint16 codepoint, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, float advance_x);

	void GrowIndex(int new_size);

	const ImFontGlyph* FindGlyph(uint16 c) const;

	void SetGlyphVisible(uint16 c, bool visible);

	const ImFontGlyph* FindGlyphNoFallback(uint16 c) const;

	bool DirtyLookupTables = true;

private:
	float m_SizePixels;
	TTF_HeadInfo m_HeadInfo;
	V_Array<int> m_GlyphsList;
	V_Array<stbrp_rect> m_Rects;
	int m_TexHeight;
	int m_TexWidth;
	River::Float2 m_TexUvScale;
	V_Array<uint8_t> m_TexPixelsAlpha8;

	V_Array<FontAtlasCustomRect> m_CustomRects;
	stbrp_context m_PackContext;

	V_Array<uint16>           IndexLookup;        // 12-16 // out //            // Sparse. Index glyphs by Unicode code-point.
	V_Array<ImFontGlyph>       Glyphs;

	int MetricsTotalSurface;

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