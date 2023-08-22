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
	auto glyph = font->m_Font->FindGlyph(/*0x2eec*/0x4fe2);

	float scale = 0.1f;
	float charWidth = glyph->AdvanceX * scale;
	float x = m_Position.x;
	float y = m_Position.y;
	if (glyph->Visible)
	{
		float x1 = x + glyph->X0 * scale;
		float x2 = x + glyph->X1 * scale;
		float y1 = y + glyph->Y0 * scale;
		float y2 = y + glyph->Y1 * scale;
		
		// Render a character
		float u1 = glyph->U0;
		float v1 = glyph->V0;
		float u2 = glyph->U1;
		float v2 = glyph->V1;

		//uint32 glyph_col = glyph->Colored ? col_untinted : col;

		uint16 vtx_index = (uint16)vertices.size();
		/*vertices.push_back(UIVertex(x1, y1, 0.0f, u1, v1, 255, 0, 0, 255));
		vertices.push_back(UIVertex(x2, y1, 0.0f, u2, v1, 255, 0, 0, 255));
		vertices.push_back(UIVertex(x2, y2, 0.0f, u2, v2, 255, 0, 0, 255));
		vertices.push_back(UIVertex(x1, y2, 0.0f, u1, v2, 255, 0, 0, 255));*/
		vertices.push_back(UIVertex(m_Position.x, m_Position.y - m_Size.y, 0.0f, u1, v2, 255, 0, 0, 255));
		vertices.push_back(UIVertex(m_Position.x, m_Position.y, 0.0f, u1, v1, 255, 0, 0, 255));
		vertices.push_back(UIVertex(m_Position.x + m_Size.x, m_Position.y, 0.0f, u2, v1, 255, 0, 0, 255));
		vertices.push_back(UIVertex(m_Position.x + m_Size.x, m_Position.y - m_Size.y, 0.0f, u2, v2, 255, 0, 0, 255));
		
		indices.push_back(vtx_index);
		indices.push_back(vtx_index + 1);
		indices.push_back(vtx_index + 2);
		indices.push_back(vtx_index);
		indices.push_back(vtx_index + 2);
		indices.push_back(vtx_index + 3);

		auto pos = m_Position;
		auto size = m_Size;
		int depth = 0;
		/*vertices.push_back(UIVertex(pos.x, pos.y - size.y, depth, 0.0f, 1.0f, 255, 0, 0, 255));
		vertices.push_back(UIVertex(pos.x, pos.y, depth, 0.0f, 0.0f));
		vertices.push_back(UIVertex(pos.x + size.x, pos.y, depth, 1.0f, 0.0f));
		vertices.push_back(UIVertex(pos.x + size.x, pos.y - size.y, depth, 1.0f, 1.0f));*/

		/*indices.push_back((uint32_t)vertices.size() - 4);
		indices.push_back((uint32_t)vertices.size() - 3);
		indices.push_back((uint32_t)vertices.size() - 2);
		indices.push_back((uint32_t)vertices.size() - 4);
		indices.push_back((uint32_t)vertices.size() - 2);
		indices.push_back((uint32_t)vertices.size() - 1);*/
	}
}
