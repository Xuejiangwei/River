#include "RiverPch.h"
#include "GUI/Header/Text.h"
#include "Application.h"
#include "Window/Header/Window.h"
#include "Renderer/Header/RHI.h"
#include "Renderer/Font/Header/FontAtlas.h"
#include "Renderer/Font/Header/Font.h"

Text::Text()
    : m_Font(nullptr), m_FontSize(10.0f)
{
}

Text::~Text()
{
}

void Text::OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices)
{
	Widget::OnRender(vertices, indices);

    UIRenderItem renderItem;
    renderItem.RenderTexture = m_Font;
    renderItem.BaseVertexLocation =  (int)vertices.size();
    renderItem.StartIndexLocation =  (int)indices.size();

	//0x2eec
	auto font = RHI::Get()->GetFont();

	auto textSize = font->m_Font->CalcTextSize(m_Text, m_FontSize);

    Float2 startPos = GetAbsoluteLeftTopPosition();
    float currPosX = startPos.x;
    float currPosY = startPos.y;
    float scale = m_FontSize / font->m_Font->GetPixelSize();
    
    int indicesCount = 0;
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
                currPosX = startPos.x;
                currPosY += m_FontSize;
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
        float height = 1.0f;
        if (glyph->Visible)
        {
            // We don't do a second finer clipping test on the Y axis as we've already skipped anything before clip_rect.y and exit once we pass clip_rect.w
            float x1 = currPosX + glyph->X0 * scale;
            float x2 = currPosX + glyph->X1 * scale;
            float y1 = currPosY + glyph->Y0 * scale;
            float y2 = currPosY + glyph->Y1 * scale;

            float w = x2 - x1;
            float h = y2 - y1;
            //if (x1 <= clip_rect.z && x2 >= clip_rect.x)
            {
                // Render a character
                float u1 = glyph->U0;
                float v1 = glyph->V0;
                float u2 = glyph->U1;
                float v2 = glyph->V1;


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

                // We are NOT calling PrimRectUV() here because non-inlined causes too much overhead in a debug builds. Inlined here:
                {
                    auto vs = indicesCount;

                    //0 1
                    //3 2
                    vertices.push_back(UIVertex(x1, y1, 0.0f, u1, v1, 255, 0, 0, 255));
                    vertices.push_back(UIVertex(x2, y1, 0.0f, u2, v1, 255, 0, 0, 255));
                    vertices.push_back(UIVertex(x2, y2, 0.0f, u2, v2, 255, 0, 0, 255));
                    vertices.push_back(UIVertex(x1, y2, 0.0f, u1, v2, 255, 0, 0, 255));

                    indices.push_back(vs);
                    indices.push_back(vs + 1);
                    indices.push_back(vs + 2);
                    indices.push_back(vs);
                    indices.push_back(vs + 2);
                    indices.push_back(vs + 3);
                
                    renderItem.IndexCount += 6;
                    indicesCount += 4;
                }
            }
        }
        currPosX += char_width;
    }

    RHI::Get()->AddUIRenderItem(renderItem);
}

void Text::SetFont(Texture* font)
{
    m_Font = font;
}

void Text::SetText(const char* text)
{
	m_Text = text;
}