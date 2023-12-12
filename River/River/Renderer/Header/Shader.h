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

	virtual void* GetShader() const = 0;

	int GetShaderId() const { return m_ShaderId; }

private:
	int m_ShaderId;
};
