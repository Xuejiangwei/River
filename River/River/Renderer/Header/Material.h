#pragma once

#include "RiverHead.h"
#include "MathStruct.h"

class Material
{
public:
	Material();

	Material(String& name);

	Material(const char* name);

	~Material();

	void InitBaseParam(const River::Float4& diffuseAlbedo, const River::Float3& fresnelR0, float roughness, int cbIndx, int diffuseSrvIndex, int normalSrvIndex);

	String Name;
	int NumFramesDirty = 3; //frame buffer num
	int MatCBIndex = -1;
	int DiffuseSrvHeapIndex = -1;
	int NormalSrvHeapIndex = -1;
	float Roughness;

	River::Float4 DiffuseAlbedo;
	River::Float3 FresnelR0;
	River::Matrix4x4 MatTransform;
};