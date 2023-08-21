#pragma once

#include "RiverHead.h"

class Texture
{
public:
	Texture(const String& name, const String& path);
	virtual ~Texture();

	String Name;
	String m_Path;
};
