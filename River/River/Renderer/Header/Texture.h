#pragma once

#include "RiverHead.h"

class Texture
{
public:
	Texture(const String& name, const String& path);
	virtual ~Texture();

protected:
	String m_Name;
	String m_Path;
};
