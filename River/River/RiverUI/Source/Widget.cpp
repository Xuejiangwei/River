#include "RiverPch.h"
#include "RiverUI/Header/Widget.h"
#include "Application.h"
#include "Window/Header/Window.h"
#include "Renderer/Header/RHI.h"

#define OUTLINE_COLOR 0, 255, 0, 255
#define OUTLINE_THICJKNESS 0.001f

Widget::Widget()
    : m_WidgetAlign(WidgetAnchors::LeftTop), m_Alignment(0.0f, 0.0f)
{
}

Widget::~Widget()
{
}

void Widget::OnUpdate(float deltaTime)
{

}

void Widget::OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices)
{
	if (RHI::Get()->IsShowUIDebugOutline())
	{
        auto [width, height] = Application::Get().GetWindow()->GetWindowSize();

        FLOAT_2 startPos = GetAbsoluteLeftTopPosition();

        UIRenderItem renderItem;
        renderItem.BaseVertexLocation = (int)vertices.size();
        renderItem.IndexCount = 6 * 4;
        renderItem.StartIndexLocation = (int)indices.size();

        float ndcStartX = startPos.x / width;
        float ndcStartY = -startPos.y / height;
        float ndcLengthX = m_Size.x / width;
        float ndcLengthY = -m_Size.y / height;

        //右手坐标系朝向屏幕里面
        //ndc (0,0) (1,0)  
        //    (0,-1) (1,-1)
        
        //0 1
        //3 2
        vertices.push_back(UIVertex(ndcStartX, ndcStartY, 0.0f, 0.0f, 0.0f, OUTLINE_COLOR));
        vertices.push_back(UIVertex(ndcStartX + OUTLINE_THICJKNESS, ndcStartY, 0.0f, 1.0f, 0.0f, OUTLINE_COLOR));
        vertices.push_back(UIVertex(ndcStartX + OUTLINE_THICJKNESS, ndcStartY + ndcLengthY, 0.0f, 1.0f, 1.0f, OUTLINE_COLOR));
        vertices.push_back(UIVertex(ndcStartX, ndcStartY + ndcLengthY, 0.0f, 0.0f, 1.0f, OUTLINE_COLOR));

        vertices.push_back(UIVertex(ndcStartX, ndcStartY, 0.0f, 0.0f, 1.0f, OUTLINE_COLOR));
        vertices.push_back(UIVertex(ndcStartX + ndcLengthX, ndcStartY, 0.0f, 0.0f, 0.0f, OUTLINE_COLOR));
        vertices.push_back(UIVertex(ndcStartX + ndcLengthX, ndcStartY - OUTLINE_THICJKNESS, 0.0f, 1.0f, 0.0f, OUTLINE_COLOR));
        vertices.push_back(UIVertex(ndcStartX, ndcStartY - OUTLINE_THICJKNESS, 0.0f, 1.0f, 1.0f, OUTLINE_COLOR));

        vertices.push_back(UIVertex(ndcStartX + ndcLengthX, ndcStartY, 0.0f, 0.0f, 0.0f, OUTLINE_COLOR));
        vertices.push_back(UIVertex(ndcStartX + ndcLengthX + OUTLINE_THICJKNESS, ndcStartY, 0.0f, 1.0f, 0.0f, OUTLINE_COLOR));
        vertices.push_back(UIVertex(ndcStartX + ndcLengthX + OUTLINE_THICJKNESS, ndcStartY + ndcLengthY, 0.0f, 1.0f, 1.0f, OUTLINE_COLOR));
        vertices.push_back(UIVertex(ndcStartX + ndcLengthX, ndcStartY + ndcLengthY, 0.0f, 0.0f, 1.0f, OUTLINE_COLOR));

        vertices.push_back(UIVertex(ndcStartX, ndcStartY + ndcLengthY, 0.0f, 0.0f, 1.0f, OUTLINE_COLOR));
        vertices.push_back(UIVertex(ndcStartX + ndcLengthX, ndcStartY + ndcLengthY, 0.0f, 0.0f, 0.0f, OUTLINE_COLOR));
        vertices.push_back(UIVertex(ndcStartX + ndcLengthX, ndcStartY + ndcLengthY - OUTLINE_THICJKNESS, 0.0f, 1.0f, 0.0f, OUTLINE_COLOR));
        vertices.push_back(UIVertex(ndcStartX, ndcStartY + ndcLengthY - OUTLINE_THICJKNESS, 0.0f, 1.0f, 1.0f, OUTLINE_COLOR));

        indices.push_back(0);
        indices.push_back(1);
        indices.push_back(2);
        indices.push_back(0);
        indices.push_back(2);
        indices.push_back(3);

        indices.push_back(0 + 4);
        indices.push_back(1 + 4);
        indices.push_back(2 + 4);
        indices.push_back(0 + 4);
        indices.push_back(2 + 4);
        indices.push_back(3 + 4);

        indices.push_back(0 + 8);
        indices.push_back(1 + 8);
        indices.push_back(2 + 8);
        indices.push_back(0 + 8);
        indices.push_back(2 + 8);
        indices.push_back(3 + 8);

        indices.push_back(0 + 12);
        indices.push_back(1 + 12);
        indices.push_back(2 + 12);
        indices.push_back(0 + 12);
        indices.push_back(2 + 12);
        indices.push_back(3 + 12);

        RHI::Get()->AddUIRenderItem(renderItem);
	}
}

void Widget::SetWidgetAlign(WidgetAnchors align)
{
    m_WidgetAlign = align;
}

void Widget::SetSize(float w, float h)
{
	m_Size.x = w;
	m_Size.y = h;
}

void Widget::SetPosition(float x, float y)
{
	m_Position.x = x;
	m_Position.y = y;
}

FLOAT_2 Widget::GetAbsoluteLeftTopPosition()
{
    switch (m_WidgetAlign)
    {
    case WidgetAnchors::LeftTop:
        return m_Position;
    case WidgetAnchors::RightTop:
        break;
    case WidgetAnchors::LeftCenter:
        break;
    case WidgetAnchors::RightCenter:
        break;
    case WidgetAnchors::Center:
        break;
    case WidgetAnchors::LeftBottom:
        break;
    case WidgetAnchors::RightBottom:
        break;
    default:
        break;
    }

    return { 0.0f, 0.0f };
}

FLOAT_2 Widget::GetWindowPosition()
{
    FLOAT_2 pos = GetAbsoluteLeftTopPosition();
    if (m_Parent)
    {
        pos += m_Parent->GetWindowPosition();
    }
    return pos;
}
