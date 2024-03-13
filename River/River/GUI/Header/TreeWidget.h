#pragma once
#include "ListWidget.h"

class TreeWidget : public ListWidget
{
public:
	TreeWidget();

	~TreeWidget();

public:
	static const String& GetWidgetTypeName()
	{
		static String typeName("TreeWidget");
		return typeName;
	}

private:
};
