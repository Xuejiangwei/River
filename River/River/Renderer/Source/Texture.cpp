#include "RiverPch.h"
#include "Texture.h"

Texture1::Texture1(const String& name, const String& path)
	: Name(std::move(name)), m_Path(std::move(path))
{
}

Texture1::~Texture1()
{
}