#include "RiverPch.h"
#include "GUI/Header/ListWidget.h"

ListWidget::ListWidget()
	: m_ChildNum(0), m_FrontIndex(0), m_BackIndex(0)
{
	m_CacheChilds.clear();
}

ListWidget::~ListWidget()
{
}

void ListWidget::OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices)
{
	Widget::OnRender(vertices, indices);

	for (size_t i = 0; i < m_CacheChilds.size(); i++)
	{
		m_CacheChilds[i]->OnRender(vertices, indices);
	}
}

void ListWidget::SetChildWidgetType(const char* type)
{
	m_ChildWidgetType = type;
}