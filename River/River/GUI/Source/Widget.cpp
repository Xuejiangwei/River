#include "RiverPch.h"
#include "GUI/Header/Widget.h"
#include "Application.h"
#include "Window/Header/Window.h"
#include "Renderer/Header/RHI.h"

#define OUTLINE_COLOR 0, 255, 0, 255
#define OUTLINE_THICJKNESS 5

Widget::Widget()
    : m_WidgetAlign(WidgetAnchors::LeftTop), m_Alignment(0.0f, 0.0f),
    m_Size({ 0, 0 }), m_Position({ 0, 0 }), m_Parent(nullptr), m_Name("")
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
        auto [width, height] = Application::Get()->GetWindow()->GetWindowSize();

        Float2 startPos = GetAbsoluteLeftTopPosition();

        UIRenderItem renderItem;
        renderItem.BaseVertexLocation = (int)vertices.size();
        renderItem.IndexCount = 6 * 4;
        renderItem.StartIndexLocation = (int)indices.size();

        //右手坐标系朝向屏幕里面

        //ndc (0,0) (1,0)  
        //    (0,-1) (1,-1)
        
        //0 1
        //3 2
        
        //左边|
        vertices.push_back(UIVertex(startPos.x, startPos.y, 0.0f, 0.0f, 0.0f, OUTLINE_COLOR));
        vertices.push_back(UIVertex(startPos.x + OUTLINE_THICJKNESS, startPos.y, 0.0f, 1.0f, 0.0f, OUTLINE_COLOR));
        vertices.push_back(UIVertex(startPos.x + OUTLINE_THICJKNESS, startPos.y + m_Size.y, 0.0f, 1.0f, 1.0f, OUTLINE_COLOR));
        vertices.push_back(UIVertex(startPos.x, startPos.y + m_Size.y, 0.0f, 0.0f, 1.0f, OUTLINE_COLOR));

        //上边——
        vertices.push_back(UIVertex(startPos.x, startPos.y, 0.0f, 0.0f, 1.0f, OUTLINE_COLOR));
        vertices.push_back(UIVertex(startPos.x + m_Size.x, startPos.y, 0.0f, 0.0f, 0.0f, OUTLINE_COLOR));
        vertices.push_back(UIVertex(startPos.x + m_Size.x, startPos.y - OUTLINE_THICJKNESS, 0.0f, 1.0f, 0.0f, OUTLINE_COLOR));
        vertices.push_back(UIVertex(startPos.x, startPos.y - OUTLINE_THICJKNESS, 0.0f, 1.0f, 1.0f, OUTLINE_COLOR));

        //右边|
        vertices.push_back(UIVertex(startPos.x + m_Size.x - OUTLINE_THICJKNESS / 2, startPos.y, 0.0f, 0.0f, 0.0f, OUTLINE_COLOR));
        vertices.push_back(UIVertex(startPos.x + m_Size.x + OUTLINE_THICJKNESS / 2, startPos.y, 0.0f, 1.0f, 0.0f, OUTLINE_COLOR));
        vertices.push_back(UIVertex(startPos.x + m_Size.x + OUTLINE_THICJKNESS / 2, startPos.y + m_Size.y, 0.0f, 1.0f, 1.0f, OUTLINE_COLOR));
        vertices.push_back(UIVertex(startPos.x + m_Size.x - OUTLINE_THICJKNESS / 2, startPos.y + m_Size.y, 0.0f, 0.0f, 1.0f, OUTLINE_COLOR));

        //下边——
        vertices.push_back(UIVertex(startPos.x, startPos.y + m_Size.y + OUTLINE_THICJKNESS / 2, 0.0f, 0.0f, 1.0f, OUTLINE_COLOR));
        vertices.push_back(UIVertex(startPos.x + m_Size.x, startPos.y + m_Size.y + OUTLINE_THICJKNESS / 2, 0.0f, 0.0f, 0.0f, OUTLINE_COLOR));
        vertices.push_back(UIVertex(startPos.x + m_Size.x, startPos.y + m_Size.y - OUTLINE_THICJKNESS / 2, 0.0f, 1.0f, 0.0f, OUTLINE_COLOR));
        vertices.push_back(UIVertex(startPos.x, startPos.y + m_Size.y - OUTLINE_THICJKNESS / 2, 0.0f, 1.0f, 1.0f, OUTLINE_COLOR));

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

Float2 Widget::GetAbsoluteLeftTopPosition()
{
    switch (m_WidgetAlign)
    {
    case WidgetAnchors::LeftTop:
        return m_Position + (m_Parent ? m_Parent->GetAbsoluteLeftTopPosition() : Float2(0.0f, 0.0f));
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