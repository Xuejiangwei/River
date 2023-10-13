#pragma once

#include "RiverHead.h"

class Texture
{
public:
	Texture(const String& name, const String& path);

	virtual ~Texture();

	virtual void SetTextureId(uint32 textureId);

	uint32 GetTextureId() const { return m_RendererId; }

	static Texture* CreateTexture(const char* name, const char* filePath);

	static Texture* CreateTexture(const char* name, int width, int height, const uint8* data);

private:
	String m_Name;
	String m_Path;

	union
	{
		uint32 m_RendererIndex;
		uint32 m_RendererId;
	};
};
