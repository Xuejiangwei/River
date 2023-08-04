#pragma once

#include "RiverHead.h"

class Texture1
{
public:
	Texture1(const String& name, const String& path);
	virtual ~Texture1();

	String Name;
	String m_Path;
};
