#pragma once
#include "Widget.h"

class ListWidget : public Widget
{
public:
	ListWidget();

	~ListWidget();

	template<typename T>
	void SetListData(V_Array<T> data)
	{
		for (size_t i = m_FrontIndex; i < m_BackIndex; i++)
		{

		}
	}

	uint8 GetChildNum() const { return m_ChildNum; }



private:
	V_Array<Share<Widget>> m_CacheChilds;
	uint8 m_ChildNum;
	uint8 m_FrontIndex;
	uint8 m_BackIndex;
};

