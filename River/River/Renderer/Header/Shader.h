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
	String Name;
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

	static Share<Shader> Create(const String& filePath);

	virtual void* GetVertexShader() = 0;

	virtual void* GetPixelShader() = 0;

private:
};
