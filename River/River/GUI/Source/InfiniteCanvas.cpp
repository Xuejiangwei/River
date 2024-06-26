#include "RiverPch.h"
#include "GUI/Header/InfiniteCanvas.h"
#include "Application.h"
#include "Input/Header/InputManager.h"
#include "Window/Header/Window.h"
#include "Renderer/Header/RHI.h"
#include "GUI/Header/Panel.h"
#include "GUI/Header/Line.h"

InfiniteCanvas::InfiniteCanvas() 
    : m_IsMousePressed(false), m_IsDefaultChildWidgetType(false)
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

    if (m_Childs.size() > 1)
    {
        for (int i = 0; i < m_Childs.size() - 1; i++)
        {
            Line line;
            line.AddPoint(m_Childs[i]->GetAbsoluteLeftTopPosition());
            line.AddPoint(m_Childs[i + 1]->GetAbsoluteLeftTopPosition());
            line.DrawLine(vertices, indices, {0, 255, 255, 255});
        }
    }
    

    Widget::OnRender(vertices, indices);

    UIRenderItem renderItem;
    renderItem.BaseVertexLocation = (int)vertices.size();
    renderItem.IndexCount = 6;
    renderItem.StartIndexLocation = (int)indices.size();

    auto [width, height] = Application::Get()->GetWindow()->GetWindowSize();
    Float2 startPos = Widget::GetAbsoluteLeftTopPosition();

    if (m_Texture)
    {
        renderItem.RenderTexture = m_Texture;
        vertices.push_back(UIVertex(startPos.x, startPos.y, 0.0f, 0.0f, 0.0f, 255, 255, 0, 255));
        vertices.push_back(UIVertex(startPos.x + m_Size.x, startPos.y, 0.0f, 1.0f, 0.0f, 255, 255, 0, 255));
        vertices.push_back(UIVertex(startPos.x + m_Size.x, startPos.y + m_Size.y, 0.0f,1.0f, 1.0f, 255, 255, 0, 255));
        vertices.push_back(UIVertex(startPos.x, startPos.y + m_Size.y, 0.0f, 0.0f, 1.0f, 255, 255, 0, 255));
    }
    else
    {
        renderItem.RenderFlag = (uint32)UIRenderItem::RenderFlag::PointColor;
        int r = m_BackgroundColor[0], g = m_BackgroundColor[1], b = m_BackgroundColor[2], a = m_BackgroundColor[3];
        vertices.push_back(UIVertex(startPos.x, startPos.y, 0.0f, 0.0f, 0.0f, r, g, b, a));
        vertices.push_back(UIVertex(startPos.x + m_Size.x, startPos.y, 0.0f, 1.0f, 0.0f, r, g, b, a));
        vertices.push_back(UIVertex(startPos.x + m_Size.x, startPos.y + m_Size.y, 0.0f, 1.0f, 1.0f, r, g, b, a));
        vertices.push_back(UIVertex(startPos.x, startPos.y + m_Size.y, 0.0f, 0.0f, 1.0f, r, g, b, a));
    }

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(0);
    indices.push_back(2);
    indices.push_back(3);

    RHI::Get()->AddUIRenderItem(renderItem);
}

bool InfiniteCanvas::OnMouseButtonDown(const MouseButtonPressedEvent& e)
{
    for (auto& widget : m_Childs)
    {
        auto panel = DynamicCast<Panel>(widget);
        if (panel && panel->OnMouseButtonDown(e))
        {
            return true;
        }
    }

    m_IsMousePressed = true;

    return true;
}

bool InfiniteCanvas::OnMouseButtonRelease(const MouseButtonReleasedEvent& e)
{
    if (m_IsMousePressed)
    {
        m_IsMousePressed = false;
        return true;
    }

    return false;
}

bool InfiniteCanvas::OnMouseMove(int mouseX, int mouseY)
{
    if (m_IsMousePressed)
    {
        auto pos = Application::Get()->GetInputManager()->GetLastMousePosition();
        pos.x -= mouseX;
        pos.y -= mouseY;

        m_MovedPosition -= pos;
        return true;
    }

    return false;
}

void InfiniteCanvas::OnMouseOut()
{
    if (m_IsMousePressed)
    {
        m_IsMousePressed = false;
    }
}

Float2 InfiniteCanvas::GetAbsoluteLeftTopPosition()
{
    switch (m_WidgetAlign)
    {
    case WidgetAnchors::LeftTop:
        return m_MovedPosition + m_Position + (m_Parent ? m_Parent->GetAbsoluteLeftTopPosition() : Float2(0.0f, 0.0f));
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

void InfiniteCanvas::SetBackgroundColor(uint8 color[4])
{
    memcpy(m_BackgroundColor, color, sizeof(m_BackgroundColor));
}

Widget* InfiniteCanvas::GetChildWidgetByName(const char* name)
{
    return nullptr;
}

Widget* InfiniteCanvas::GetChildWidget(int index)
{
    if (m_Childs.size() < index)
    {
        return nullptr;
    }

    return m_Childs[index].get();
}

void InfiniteCanvas::SetChildWidgetType(const char* type)
{
    m_ChildWidgetType = type;
    m_IsDefaultChildWidgetType = m_ChildWidgetType.find(".json") == std::string::npos;
}

void InfiniteCanvas::CreateChildWidget(int count)
{
    for (int i = 0; i < count; i++)
    {
        auto widget = GuiManager::CreateWidgetByTypeName(m_ChildWidgetType, this);
        m_Childs.push_back(widget);

        auto panel = DynamicCast<Panel>(widget);
        auto size = panel->GetSize();
        panel->SetPosition(i * size.x, i * size.y);
        //auto text = dynamic_cast<Text*>(panel->GetChildWidgetByName("TextTitle"));
        //text->SetText(data[i]);
    }
}
