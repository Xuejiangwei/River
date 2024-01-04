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
    //MetricsTotalSurface += (int)((glyph.U1 - glyph.U0) * m_Atlas->m_TextureWidth + pad) * (int)((glyph.V1 - glyph.V0) * m_Atlas->m_TextureHeight + pad);
}

Float2 Font::CalcTextSize(const String& text, float size)
{
    Float2 text_size;
    float fontSize = m_SizePixels;
    float line_width = 0.0f;
    float line_height = fontSize;
    const float scale = size / fontSize;

    for (auto s = text.begin(); s != text.end(); )
    {
        const char* prev_s = s._Unwrapped();
        unsigned int c = (unsigned int)*s;
        if (c < 0x80)
            s += 1;
        else
            s += ImTextCharFromUtf8(&c, s._Unwrapped(), text.end()._Unwrapped());

        if (c < 32)
        {
            if (c == '\n')
            {
                text_size.x = std::max(text_size.x, line_width);
                text_size.y += line_height;
                line_width = 0.0f;
                continue;
            }
            if (c == '\r')
                continue;
        }

        const float char_width = ((int)c < IndexAdvanceX.size() ? IndexAdvanceX[c] : FallbackAdvanceX) * scale;
        if (line_width + char_width >= FLT_MAX)
        {
            break;
        }

        line_width += char_width;
    }

    if (text_size.x < line_width)
        text_size.x = line_width;

    if (line_width > 0 || text_size.y == 0.0f)
        text_size.y += line_height;

    return text_size;
}

int Font::ImTextCharFromUtf8(unsigned int* out_char, const char* in_text, const char* in_text_end)
{
#define IM_UNICODE_CODEPOINT_MAX 0xFFFF
#define IM_UNICODE_CODEPOINT_INVALID  0xFFFD

    static const char lengths[32] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0 };
    static const int masks[] = { 0x00, 0x7f, 0x1f, 0x0f, 0x07 };
    static const uint32_t mins[] = { 0x400000, 0, 0x80, 0x800, 0x10000 };
    static const int shiftc[] = { 0, 18, 12, 6, 0 };
    static const int shifte[] = { 0, 6, 4, 2, 0 };
    int len = lengths[*(const unsigned char*)in_text >> 3];
    int wanted = len + (len ? 0 : 1);

    if (in_text_end == NULL)
        in_text_end = in_text + wanted; // Max length, nulls will be taken into account.

    // Copy at most 'len' bytes, stop copying at 0 or past in_text_end. Branch predictor does a good job here,
    // so it is fast even with excessive branching.
    unsigned char s[4];
    s[0] = in_text + 0 < in_text_end ? in_text[0] : 0;
    s[1] = in_text + 1 < in_text_end ? in_text[1] : 0;
    s[2] = in_text + 2 < in_text_end ? in_text[2] : 0;
    s[3] = in_text + 3 < in_text_end ? in_text[3] : 0;

    // Assume a four-byte character and load four bytes. Unused bits are shifted out.
    *out_char = (uint32_t)(s[0] & masks[len]) << 18;
    *out_char |= (uint32_t)(s[1] & 0x3f) << 12;
    *out_char |= (uint32_t)(s[2] & 0x3f) << 6;
    *out_char |= (uint32_t)(s[3] & 0x3f) << 0;
    *out_char >>= shiftc[len];

    // Accumulate the various error conditions.
    int e = 0;
    e = (*out_char < mins[len]) << 6; // non-canonical encoding
    e |= ((*out_char >> 11) == 0x1b) << 7;  // surrogate half?
    e |= (*out_char > IM_UNICODE_CODEPOINT_MAX) << 8;  // out of range?
    e |= (s[1] & 0xc0) >> 2;
    e |= (s[2] & 0xc0) >> 4;
    e |= (s[3]) >> 6;
    e ^= 0x2a; // top two bits of each tail byte correct?
    e >>= shifte[len];

    if (e)
    {
        // No bytes are consumed when *in_text == 0 || in_text == in_text_end.
        // One byte is consumed in case of invalid first byte of in_text.
        // All available bytes (at most `len` bytes) are consumed on incomplete/invalid second to last bytes.
        // Invalid or incomplete input may consume less bytes than wanted, therefore every byte has to be inspected in s.
        wanted = std::min(wanted, !!s[0] + !!s[1] + !!s[2] + !!s[3]);
        *out_char = IM_UNICODE_CODEPOINT_INVALID;
    }

    return wanted;
}
