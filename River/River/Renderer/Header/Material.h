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

	void InitBaseParam(const River::Float4& diffuseAlbedo, const River::Float3& fresnelR0, float roughness, int cbIndx, int srvIndex);

	String m_Name;
	float m_Roughness;
	River::Float4 m_DiffuseAlbedo;
	River::Float3 m_FresnelR0;
	River::Matrix4x4 m_MatTransform;

	int MatCBIndex = -1;
	int DiffuseSrvHeapIndex = -1;
	int NormalSrvHeapIndex = -1;
	int NumFramesDirty = 3; //frame buffer num
};