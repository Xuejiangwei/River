#include "RiverPch.h"
#include "Material.h"
#include "RHI.h"

Material::Material(String& name)
	: m_Name(std::move(name)), DiffuseAlbedo({ 1.0f, 1.0f, 1.0f, 1.0f }), FresnelR0({ 0.01f,0.01f,0.01f }), Roughness(0.25f)
	, MatTransform(River::Matrix4x4::UnitMatrix()), m_DiffuseTexture(nullptr), m_NormalTexture(nullptr)
{
	RHI::Get()->SetUpMaterial(this);
}

Material::Material(const char* name)
	: Material(String(name))
{
	RHI::Get()->SetUpMaterial(this);
}

Material::~Material()
{
}

Material* Material::CreateMaterial(const char* name)
{
	return RHI::Get()->CreateMaterial(name);
}

void Material::InitBaseParam(MaterialBlendMode blendMode, const River::Float4& diffuseAlbedo, const River::Float3& fresnelR0, float roughness, int cbIndx,
	Texture* diffuseSrvIndex, Texture* normalSrvIndex)
{
	m_BlendMode = blendMode;
	DiffuseAlbedo = diffuseAlbedo;
	FresnelR0 = fresnelR0;
	Roughness = roughness;
	MatCBIndex = cbIndx;
	m_DiffuseTexture = diffuseSrvIndex;
	m_NormalTexture = normalSrvIndex;
}
