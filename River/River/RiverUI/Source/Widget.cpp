#include "RiverPch.h"
#include "RiverUI/Header/Widget.h"
#include "RiverUI/Header/UIRenderItem.h"

Widget::Widget()
{
	m_RenderItem = MakeUnique<UIRenderItem>(this);
}

Widget::~Widget()
{
}

void Widget::OnUpdate(float deltaTime)
{

}

void Widget::OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices)
{
	m_RenderItem->OnRender(vertices, indices);
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
