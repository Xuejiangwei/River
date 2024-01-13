#pragma once

#include "RiverHead.h"

class Texture
{
public:
	enum class Type
	{
		Texture2D,
		CubeTexture
	};

public:
	Texture(const String& name, const String& path, Type type);

	virtual ~Texture();

	virtual void SetTextureId(uint32 textureId);

	bool IsCubeTexture() const { return m_Type == Type::CubeTexture; }

	uint64 GetTextureHandle() const { return m_DescriptorHandle; }

	uint32 GetTextureId() const { return m_RendererId; }

	static Texture* CreateTexture(const char* name, const char* filePath, bool isImmediately = false);

	static Texture* CreateCubeTexture(const char* name, const char* filePath, bool isImmediately = false);

	static Texture* CreateTexture(const char* name, int width, int height, const uint8* data);

	static Texture* CreateTexture(const char* name, int width, int height);

protected:
	String m_Name;
	String m_Path;
	Type m_Type;
	union
	{
		uint64 m_DescriptorHandle;
		uint32 m_RendererId;
	};
};
