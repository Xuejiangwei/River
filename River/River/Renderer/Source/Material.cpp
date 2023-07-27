#include "RiverPch.h"
#include "Material.h"

Material::Material()
	: Material("")
{
}

Material::Material(String& name)
	: m_Name(std::move(name)), m_DiffuseAlbedo({ 1.0f, 1.0f, 1.0f, 1.0f }), m_FresnelR0({ 0.01f,0.01f,0.01f }), m_Roughness(0.25f)
	, m_MatTransform(River::Matrix4x4::UnitMatrix())
{
}

Material::Material(const char* name)
	: m_Name(name), m_DiffuseAlbedo({ 1.0f, 1.0f, 1.0f, 1.0f }), m_FresnelR0({ 0.01f,0.01f,0.01f }), m_Roughness(0.25f)
	, m_MatTransform(River::Matrix4x4::UnitMatrix())
{
}

Material::~Material()
{
}

void Material::InitBaseParam(const River::Float4& diffuseAlbedo, const River::Float3& fresnelR0, float roughness, int cbIndx, int srvIndex)
{
	m_DiffuseAlbedo = diffuseAlbedo;
	m_FresnelR0 = fresnelR0;
	m_Roughness = roughness;
	MatCBIndex = cbIndx;
	DiffuseSrvHeapIndex = srvIndex;
}
