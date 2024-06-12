#include "RiverPch.h"
#include "GUI/Header/Canvas.h"
#include "Application.h"
#include "Window/Header/Window.h"
#include "Renderer/Header/RHI.h"

Canvas::Canvas()
{
    memset(m_BackgroundColor, 0, sizeof(m_BackgroundColor));
}

Canvas::~Canvas()
{
}

void Canvas::OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices)
{
	Widget::OnRender(vertices, indices);

    UIRenderItem renderItem;
    renderItem.BaseVertexLocation = (int)vertices.size();
    renderItem.IndexCount = 6;
    renderItem.StartIndexLocation = (int)indices.size();

    auto [width, height] = Application::Get()->GetWindow()->GetWindowSize();
    Float2 startPos = GetAbsoluteLeftTopPosition();

    int r = m_BackgroundColor[0], g = m_BackgroundColor[1], b = m_BackgroundColor[2], a = m_BackgroundColor[3];
    vertices.push_back(UIVertex(startPos.x, startPos.y, 0.0f, 0.0f, 0.0f, r, g, b, a));
    vertices.push_back(UIVertex(startPos.x + m_Size.x, startPos.y, 0.0f, 1.0f, 0.0f, r, g, b, a));
    vertices.push_back(UIVertex(startPos.x + m_Size.x, startPos.y + m_Size.y, 0.0f, 1.0f, 1.0f, r, g, b, a));
    vertices.push_back(UIVertex(startPos.x, startPos.y + m_Size.y, 0.0f, 0.0f, 1.0f, r, g, b, a));

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(0);
    indices.push_back(2);
    indices.push_back(3);

    renderItem.RenderFlag = (uint32)UIRenderItem::RenderFlag::PointColor;
    RHI::Get()->AddUIRenderItem(renderItem);
}

void Canvas::SetBackgroundColor(uint8 color[4])
{
	memcpy(m_BackgroundColor, color, sizeof(m_BackgroundColor));
}