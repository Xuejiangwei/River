#include "RiverPch.h"
#include "Texture.h"

Texture::Texture(const String& name, const String& path)
	: m_Name(std::move(name)), m_Path(std::move(path))
{
}

Texture::~Texture()
{
}

void Texture::SetTextureId(uint32 textureId)
{
	m_RendererId = textureId;
}
