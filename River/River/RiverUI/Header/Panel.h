#pragma once

#include "RiverHead.h"
#include "Widget.h"

class Panel : public Widget
{
public:
	Panel();

	virtual ~Panel() override;

	Panel& operator[](Share<Widget> widget)
	{
		m_Children.push_back(widget);
		return *this;
	}

private:
	V_Array<Share<Widget>> m_Children;
};
