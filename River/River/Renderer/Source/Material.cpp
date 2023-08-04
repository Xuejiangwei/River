#include "RiverPch.h"
#include "Material.h"

Material::Material()
	: Material("")
{
}

Material::Material(String& name)
	: Name(std::move(name)), DiffuseAlbedo({ 1.0f, 1.0f, 1.0f, 1.0f }), FresnelR0({ 0.01f,0.01f,0.01f }), Roughness(0.25f)
	, MatTransform(River::Matrix4x4::UnitMatrix())
{
}

Material::Material(const char* name)
	: Name(name), DiffuseAlbedo({ 1.0f, 1.0f, 1.0f, 1.0f }), FresnelR0({ 0.01f,0.01f,0.01f }), Roughness(0.25f)
	, MatTransform(River::Matrix4x4::UnitMatrix())
{
}

Material::~Material()
{
}

void Material::InitBaseParam(const River::Float4& diffuseAlbedo, const River::Float3& fresnelR0, float roughness, int cbIndx, int diffuseSrvIndex, int normalSrvIndex)
{
	DiffuseAlbedo = diffuseAlbedo;
	FresnelR0 = fresnelR0;
	Roughness = roughness;
	MatCBIndex = cbIndx;
	DiffuseSrvHeapIndex = diffuseSrvIndex;
	NormalSrvHeapIndex = normalSrvIndex;
}
