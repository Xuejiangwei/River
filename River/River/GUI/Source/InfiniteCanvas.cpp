#include "RiverPch.h"
#include "GUI/Header/InfiniteCanvas.h"
#include "Application.h"
#include "Window/Header/Window.h"
#include "Renderer/Header/RHI.h"

InfiniteCanvas::InfiniteCanvas()
{
    memset(m_BackgroundColor, 0, sizeof(m_BackgroundColor));
}

InfiniteCanvas::~InfiniteCanvas()
{
}

void InfiniteCanvas::OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices)
{
    for (auto& child : m_Childs)
    {
        child->OnRender(vertices, indices);
    }

    Widget::OnRender(vertices, indices);

    UIRenderItem renderItem;
    renderItem.BaseVertexLocation = (int)vertices.size();
    renderItem.IndexCount = 6;
    renderItem.StartIndexLocation = (int)indices.size();

    auto [width, height] = Application::Get()->GetWindow()->GetWindowSize();
    Float2 startPos = GetAbsoluteLeftTopPosition();
    float ndcStartX = startPos.x / width;
    float ndcStartY = -startPos.y / height;
    float ndcLengthX = m_Size.x / width * 2;
    float ndcLengthY = -m_Size.y / height * 2;

    ndcStartX = ndcStartX * 2 - 1;
    ndcStartY = ndcStartY * 2 + 1;

    int r = m_BackgroundColor[0], g = m_BackgroundColor[1], b = m_BackgroundColor[2], a = m_BackgroundColor[3];
    vertices.push_back(UIVertex(ndcStartX, ndcStartY, 0.0f, 0.0f, 0.0f, r, g, b, a));
    vertices.push_back(UIVertex(ndcStartX + ndcLengthX, ndcStartY, 0.0f, 1.0f, 0.0f, r, g, b, a));
    vertices.push_back(UIVertex(ndcStartX + ndcLengthX, ndcStartY + ndcLengthY, 0.0f, 1.0f, 1.0f, r, g, b, a));
    vertices.push_back(UIVertex(ndcStartX, ndcStartY + ndcLengthY, 0.0f, 0.0f, 1.0f, r, g, b, a));

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(0);
    indices.push_back(2);
    indices.push_back(3);

    renderItem.RenderFlag = (uint32)UIRenderItem::RenderFlag::PointColor;
    RHI::Get()->AddUIRenderItem(renderItem);
}

void InfiniteCanvas::SetBackgroundColor(uint8 color[4])
{
    memcpy(m_BackgroundColor, color, sizeof(m_BackgroundColor));
}

void InfiniteCanvas::SetChildWidgetType(const char* type)
{
    m_ChildWidgetType = type;
    m_IsDefaultChildWidgetType = m_ChildWidgetType.find(".json") == std::string::npos;
}