#include "RiverPch.h"
#include "Component/Header/LightComponent.h"
#include "Math/Header/Geometric.h"

LightComponent::LightComponent()
	: m_Rotation(0.f)
{
}

LightComponent::~LightComponent()
{
}

void LightComponent::Tick(float deltaTime)
{
	m_Rotation = 0.1f * deltaTime;
	Matrix4x4 rot = Matrix4x4_RotationY(m_Rotation);

	Float4 lightDir = GetFloat3(m_Direction);
	m_CurrDirection = Vector3TransformNormal(lightDir, rot);
}
