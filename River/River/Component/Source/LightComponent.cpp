#include "RiverPch.h"
#include "Component/Header/LightComponent.h"
#include "Math/Header/Geometric.h"

LightComponent::LightComponent(Float3 direction)
	: m_Rotation(0.f), m_Direction(direction)
{

}

LightComponent::~LightComponent()
{
}

void LightComponent::Tick(float deltaTime)
{
	m_Rotation = 1;// += 0.1f * deltaTime;
	Matrix4x4 rot = Matrix4x4_RotationY(m_Rotation);

	Float4 lightDir = GetFloat3(m_Direction);
	m_CurrDirection = Vector3TransformNormal(lightDir, rot);
}

void LightComponent::SetLightStrength(const Float3& strength)
{
	m_LightStrength = strength;
}
