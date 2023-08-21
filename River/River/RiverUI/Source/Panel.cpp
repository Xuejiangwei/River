#include "RiverPch.h"
#include "RiverUI/Header/Panel.h"

Panel::Panel()
{
}

Panel::~Panel()
{
}

void Panel::OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices)
{
	Widget::OnRender(vertices, indices);

	for (size_t i = 0; i < m_Children.size(); i++)
	{
		m_Children[i]->OnRender(vertices, indices);
	}
}
