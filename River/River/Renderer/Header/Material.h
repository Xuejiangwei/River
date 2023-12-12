#pragma once

#include "RiverHead.h"
#include "MathStruct.h"

class Texture;

enum class MaterialBlendMode
{
	Opaque,
	Translucent
};

class Material
{
public:
	Material(String& name);

	Material(const char* name);

	~Material();

	static Material* CreateMaterial(const char* name);

	void InitBaseParam(MaterialBlendMode blendMode, const River::Float4& diffuseAlbedo, const River::Float3& fresnelR0, float roughness, int cbIndx,
		Texture* diffuseSrvIndex, Texture* normalSrvIndex);

public:
	String m_Name;
	MaterialBlendMode m_BlendMode;
	int m_RefShaderId;
	int NumFramesDirty = 3; //frame buffer num
	int MatCBIndex = -1;
	float Roughness;

	River::Float4 DiffuseAlbedo;
	River::Float3 FresnelR0;
	River::Matrix4x4 MatTransform;

	Texture* m_DiffuseTexture;
	Texture* m_NormalTexture;
};