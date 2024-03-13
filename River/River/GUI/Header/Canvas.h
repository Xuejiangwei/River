#pragma once
#include "Widget.h"

class Canvas : public Widget
{
public:
	Canvas();

	~Canvas();

public:
	static const String& GetWidgetTypeName()
	{
		static String typeName("Canvas");
		return typeName;
	}

private:

};