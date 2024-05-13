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

enum class CullMode
{
	None = 1,
	Front,
	Back,
};

enum class ComparisonFunc
{
	Never = 1,
	Less,
	Equal,
	LessEqual,
	Greater,
	NotEqual,
	GreaterEqual,
	always,
};

enum class ShaderDefaultType : uint8
{
	None,
	ShadowMap,
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

struct ShaderDefine
{
	const char* Name;
	const char* Definition;
};

struct ShaderParam
{
	CullMode Cull;
	ComparisonFunc DepthComparisonFunc;
	ShaderDefaultType DefaultType = ShaderDefaultType::None;
};

class Shader
{
public:
	Shader();
	
	virtual ~Shader();

	virtual void* GetVSShader() const = 0;
	
	virtual void* GetPSShader() const = 0;

	int GetShaderId() const { return m_ShaderId; }

public:
	static Shader* CreateShader(const String& name, const String& path, Pair<const ShaderDefine*, const ShaderDefine*> defines = { nullptr, nullptr },
		ShaderParam* param = nullptr);

private:
	int m_ShaderId;
};
