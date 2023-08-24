#include "RiverPch.h"
#include "RiverUI/Header/Text.h"
#include "Renderer/Header/RHI.h"
#include "Renderer/Font/Header/FontAtlas.h"
#include "Renderer/Font/Header/Font.h"

Text::Text()
{
}

Text::~Text()
{
}

void Text::OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices)
{
	Widget::OnRender(vertices, indices);

	//0x2eec
	auto font = RHI::Get()->GetFont();

	auto textSize = font->m_Font->CalcTextSize(m_Text, m_FontSize);
    float scale = 1;
    float x = m_Position.x;
    float y = m_Position.y;
   
    scale = m_FontSize / 720;
    auto s = m_Text.begin();
    while (s != m_Text.end())
    {
        // Decode and advance source
        unsigned int c = (unsigned int)*s;
        if (c < 0x80)
            s += 1;
        else
            s += Font::ImTextCharFromUtf8(&c, s._Unwrapped(), m_Text.end()._Unwrapped());

        if (c < 32)
        {
            if (c == '\n')
            {
                x = m_Position.x;
                y += m_FontSize * scale;
                //if (y > clip_rect.w)
                //    break; // break out of main loop
                continue;
            }
            if (c == '\r')
                continue;
        }

        const ImFontGlyph* glyph = font->m_Font->FindGlyph((uint16)c);
        if (glyph == NULL)
            continue;

        float char_width = glyph->AdvanceX * scale;
        if (glyph->Visible)
        {
            // We don't do a second finer clipping test on the Y axis as we've already skipped anything before clip_rect.y and exit once we pass clip_rect.w
            float x1 = x + glyph->X0 * scale;
            float x2 = x + glyph->X1 * scale;
            float y1 = y + glyph->Y0 * scale;
            float y2 = y + glyph->Y1 * scale;
            //if (x1 <= clip_rect.z && x2 >= clip_rect.x)
            {
                // Render a character
                float u1 = glyph->U0;
                float v1 = glyph->V0;
                float u2 = glyph->U1;
                float v2 = glyph->V1;

                FLOAT_2 ts = { 0.05f, 0.05f };

                // CPU side clipping used to fit text in their frame when the frame is too small. Only does clipping for axis aligned quads.
                /* if (cpu_fine_clip)
                {
                    if (x1 < clip_rect.x)
                    {
                        u1 = u1 + (1.0f - (x2 - clip_rect.x) / (x2 - x1)) * (u2 - u1);
                        x1 = clip_rect.x;
                    }
                    if (y1 < clip_rect.y)
                    {
                        v1 = v1 + (1.0f - (y2 - clip_rect.y) / (y2 - y1)) * (v2 - v1);
                        y1 = clip_rect.y;
                    }
                    if (x2 > clip_rect.z)
                    {
                        u2 = u1 + ((clip_rect.z - x1) / (x2 - x1)) * (u2 - u1);
                        x2 = clip_rect.z;
                    }
                    if (y2 > clip_rect.w)
                    {
                        v2 = v1 + ((clip_rect.w - y1) / (y2 - y1)) * (v2 - v1);
                        y2 = clip_rect.w;
                    }
                    if (y1 >= y2)
                    {
                        x += char_width;
                        continue;
                    }
                }*/

                // Support for untinted glyphs
                uint32 glyph_col = River::RGBA32(255, 0, 0, 255); //glyph->Colored ? col_untinted : col;

                // We are NOT calling PrimRectUV() here because non-inlined causes too much overhead in a debug builds. Inlined here:
                {
                    auto vs = (uint16)vertices.size();

                    vertices.push_back(UIVertex(x, m_Position.y - ts.y, 0.0f, u1, v2, 255, 0, 0, 255));
                    vertices.push_back(UIVertex(x, m_Position.y, 0.0f, u1, v1, 255, 0, 0, 255));
                    vertices.push_back(UIVertex(x + ts.x, m_Position.y, 0.0f, u2, v1, 255, 0, 0, 255));
                    vertices.push_back(UIVertex(x + ts.x, m_Position.y - ts.y, 0.0f, u2, v2, 255, 0, 0, 255));

                    indices.push_back(vs);
                    indices.push_back(vs + 1);
                    indices.push_back(vs + 2);
                    indices.push_back(vs);
                    indices.push_back(vs + 2);
                    indices.push_back(vs + 3);
                }
            }
        }
        x += char_width;
    }
}

void Text::SetText(const char* text)
{
	m_Text = text;
}