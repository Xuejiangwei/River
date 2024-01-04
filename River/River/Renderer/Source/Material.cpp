#include "RiverPch.h"
#include "Material.h"
#include "AssetManager.h"

Material::Material(String& name)
	: m_Name(std::move(name)), DiffuseAlbedo({ 1.0f, 1.0f, 1.0f, 1.0f }), FresnelR0({ 0.01f,0.01f,0.01f }), Roughness(0.25f)
	, MatTransform(Matrix4x4::UnitMatrix()), m_DiffuseTexture(nullptr), m_NormalTexture(nullptr), m_Shader(nullptr)
{
}

Material::Material(const char* name)
	: Material(String(name))
{
}

Material::~Material()
{
}

Material* Material::CreateMaterial(const char* name, const char* shaderName)
{
	auto material = MakeUnique<Material>(name);
	auto ret = material.get();

	AssetManager::Get()->AddCacheMaterial(name, material);

	return ret;
}

void Material::InitBaseParam(MaterialBlendMode blendMode, Shader* shader, const Float4& diffuseAlbedo, const Float3& fresnelR0, float roughness, int cbIndx,
	Texture* diffuseSrvIndex, Texture* normalSrvIndex)
{
	m_BlendMode = blendMode;
	m_Shader = shader;
	DiffuseAlbedo = diffuseAlbedo;
	FresnelR0 = fresnelR0;
	Roughness = roughness;
	MatCBIndex = cbIndx;
	m_DiffuseTexture = diffuseSrvIndex;
	m_NormalTexture = normalSrvIndex;
}
