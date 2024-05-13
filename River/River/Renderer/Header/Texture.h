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

	virtual int GetTextureWidth() = 0;

	virtual int GetTextureHeight() = 0;

	bool IsCubeTexture() const { return m_Type == Type::CubeTexture; }

	uint64 GetTextureHandle() const { return m_DescriptorHandle; }

	uint32 GetTextureId() const { return m_RendererId; }

	static Texture* CreateTexture(const String& name, const String& filePath, bool isImmediately = false);

	static Texture* CreateCubeTexture(const String& name, const String& filePath, bool isImmediately = false);

	static Texture* CreateTexture(const String& name, int width, int height, const uint8* data);

	static Texture* CreateTexture(const String& name, int width, int height);

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
