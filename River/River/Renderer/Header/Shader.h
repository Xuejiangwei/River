#pragma once

#include "RiverHead.h"

enum class ShaderLayotFormat
{
	None,
	R32_G32_B32_A32_TYPELESS,
	R32_G32_B32_A32_FLOAT,
	R32_G32_B32_A32_UINT
};

enum class ShaderLayotClassification
{
	PER_VERTEX_DATA,
	PER_INSTANCE_DATA
};

struct ShaderLayout
{
	String m_Name;
	int Index;
	ShaderLayotFormat Format;
	int Slot;
	int OffsetByte;
	ShaderLayotClassification Classification;
};

class Shader
{
public:
	Shader();
	
	virtual ~Shader();

	virtual void* GetVSShader() const = 0;
	
	virtual void* GetPSShader() const = 0;

	static Shader* CreateShader(const char* name, const char* path);

	int GetShaderId() const { return m_ShaderId; }

private:
	int m_ShaderId;
};
