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