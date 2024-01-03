#pragma once

#include "RiverHead.h"

class Texture
{
public:
	Texture(const String& name, const String& path);

	virtual ~Texture();

	virtual void SetTextureId(uint32 textureId);

	uint64 GetTextureHandle() const { return m_DescriptorHandle; }

	uint32 GetTextureId() const { return m_RendererId; }

	static Texture* CreateTexture(const char* name, const char* filePath, bool isImmediately = false);

	static Texture* CreateTexture(const char* name, int width, int height, const uint8* data);

	static Texture* CreateTextureWithResource(const char* name, void* resoure);

protected:
	String m_Name;
	String m_Path;

	union
	{
		uint64 m_DescriptorHandle;
		uint32 m_RendererId;
	};
};
