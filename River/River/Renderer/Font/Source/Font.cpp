#include "RiverPch.h"
#include "Renderer/Font/Header/Font.h"
#include "Renderer/Font/Header/FontAtlas.h"
#include "Application.h"

#define IM_ARRAYSIZE(_ARR)          ((int)(sizeof(_ARR) / sizeof(*(_ARR))))     // Size of a static C-style array. Don't use on pointers!


static uint16 FindFirstExistingGlyph(Font* font, const uint16* candidate_chars, int candidate_chars_count)
{
    for (int n = 0; n < candidate_chars_count; n++)
        if (font->FindGlyphNoFallback(candidate_chars[n]) != NULL)
            return candidate_chars[n];
    return (uint16)-1;
}

Font::Font(const char* path, class FontAtlas* atlas, float pixelSize)
    : m_SizePixels(pixelSize), m_Atlas(atlas)
{

}

Font::~Font()
{

}

void Font::GrowIndex(int new_size)
{
    assert(IndexAdvanceX.size() == IndexLookup.size());
    if (new_size <= IndexLookup.size())
        return;
    IndexAdvanceX.resize(new_size, -1.0f);
    IndexLookup.resize(new_size, (uint16)-1);
}

const ImFontGlyph* Font::FindGlyph(uint16 c) const
{
    if (c >= (size_t)IndexLookup.size())
        return FallbackGlyph;
    const uint16 i = IndexLookup[c];
    if (i == (uint16)-1)
        return FallbackGlyph;
    return &Glyphs[i];
}

void Font::SetGlyphVisible(uint16 c, bool visible)
{
    if (ImFontGlyph* glyph = (ImFontGlyph*)(void*)FindGlyph((uint16)c))
        glyph->Visible = visible ? 1 : 0;
}

const ImFontGlyph* Font::FindGlyphNoFallback(uint16 c) const
{
    if (c >= (size_t)IndexLookup.size())
        return NULL;
    const uint16 i = IndexLookup[c];
    if (i == (uint16)-1)
        return NULL;
    return &Glyphs[i];
}

void Font::BuildLookupTable()
{
    int max_codepoint = 0;
    for (int i = 0; i != Glyphs.size(); i++)
        max_codepoint = std::max(max_codepoint, (int)Glyphs[i].Codepoint);

    // Build lookup table
    assert(Glyphs.size() < 0xFFFF); // -1 is reserved
    IndexAdvanceX.clear();
    IndexLookup.clear();
    DirtyLookupTables = false;
    memset(Used4kPagesMap, 0, sizeof(Used4kPagesMap));
    GrowIndex(max_codepoint + 1);
    for (int i = 0; i < Glyphs.size(); i++)
    {
        int codepoint = (int)Glyphs[i].Codepoint;
        IndexAdvanceX[codepoint] = Glyphs[i].AdvanceX;
        IndexLookup[codepoint] = (uint16)i;

        // Mark 4K page as used
        const int page_n = codepoint / 4096;
        Used4kPagesMap[page_n >> 3] |= 1 << (page_n & 7);
    }

    // Create a glyph to handle TAB
    // FIXME: Needs proper TAB handling but it needs to be contextualized (or we could arbitrary say that each string starts at "column 0" ?)
    if (FindGlyph((uint16)' '))
    {
        if (Glyphs.back().Codepoint != '\t')   // So we can call this function multiple times (FIXME: Flaky)
            Glyphs.resize(Glyphs.size() + 1);
        ImFontGlyph& tab_glyph = Glyphs.back();
        tab_glyph = *FindGlyph((uint16)' ');
        tab_glyph.Codepoint = '\t';
        tab_glyph.AdvanceX *= 4;
        IndexAdvanceX[(int)tab_glyph.Codepoint] = (float)tab_glyph.AdvanceX;
        IndexLookup[(int)tab_glyph.Codepoint] = (uint16)(Glyphs.size() - 1);
    }

    // Mark special glyphs as not visible (note that AddGlyph already mark as non-visible glyphs with zero-size polygons)
    SetGlyphVisible((uint16)' ', false);
    SetGlyphVisible((uint16)'\t', false);

    // Setup Fallback character
    const uint16 fallback_chars[] = { (uint16)0xFFFD, (uint16)'?', (uint16)' ' };
    FallbackGlyph = FindGlyphNoFallback(FallbackChar);
    if (FallbackGlyph == NULL)
    {
        FallbackChar = FindFirstExistingGlyph(this, fallback_chars, IM_ARRAYSIZE(fallback_chars));
        FallbackGlyph = FindGlyphNoFallback(FallbackChar);
        if (FallbackGlyph == NULL)
        {
            FallbackGlyph = &Glyphs.back();
            FallbackChar = (uint16)FallbackGlyph->Codepoint;
        }
    }
    FallbackAdvanceX = FallbackGlyph->AdvanceX;
    for (int i = 0; i < max_codepoint + 1; i++)
        if (IndexAdvanceX[i] < 0.0f)
            IndexAdvanceX[i] = FallbackAdvanceX;

    // Setup Ellipsis character. It is required for rendering elided text. We prefer using U+2026 (horizontal ellipsis).
    // However some old fonts may contain ellipsis at U+0085. Here we auto-detect most suitable ellipsis character.
    // FIXME: Note that 0x2026 is rarely included in our font ranges. Because of this we are more likely to use three individual dots.
    const uint16 ellipsis_chars[] = { (uint16)0x2026, (uint16)0x0085 };
    const uint16 dots_chars[] = { (uint16)'.', (uint16)0xFF0E };
    if (EllipsisChar == (uint16)-1)
        EllipsisChar = FindFirstExistingGlyph(this, ellipsis_chars, IM_ARRAYSIZE(ellipsis_chars));
    const uint16 dot_char = FindFirstExistingGlyph(this, dots_chars, IM_ARRAYSIZE(dots_chars));
    if (EllipsisChar != (uint16)-1)
    {
        EllipsisCharCount = 1;
        EllipsisWidth = EllipsisCharStep = FindGlyph(EllipsisChar)->X1;
    }
    else if (dot_char != (uint16)-1)
    {
        const ImFontGlyph* glyph = FindGlyph(dot_char);
        EllipsisChar = dot_char;
        EllipsisCharCount = 3;
        EllipsisCharStep = (glyph->X1 - glyph->X0) + 1.0f;
        EllipsisWidth = EllipsisCharStep * 3.0f - 1.0f;
    }
}

void Font::AddGlyph(uint16 codepoint, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, float advance_x)
{
    Glyphs.resize(Glyphs.size() + 1);
    ImFontGlyph& glyph = Glyphs.back();
    glyph.Codepoint = (unsigned int)codepoint;
    glyph.Visible = (x0 != x1) && (y0 != y1);
    glyph.Colored = false;
    glyph.X0 = x0;
    glyph.Y0 = y0;
    glyph.X1 = x1;
    glyph.Y1 = y1;
    glyph.U0 = u0;
    glyph.V0 = v0;
    glyph.U1 = u1;
    glyph.V1 = v1;
    glyph.AdvanceX = advance_x;

    // Compute rough surface usage metrics (+1 to account for average padding, +0.99 to round)
    // We use (U1-U0)*TexWidth instead of X1-X0 to account for oversampling.
    float pad = 1 + 0.99f;
    DirtyLookupTables = true;
    MetricsTotalSurface += (int)((glyph.U1 - glyph.U0) * m_Atlas->m_TextureWidth + pad) * (int)((glyph.V1 - glyph.V0) * m_Atlas->m_TextureHeight + pad);
}
