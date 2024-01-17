#include "RiverPch.h"
#include "Component/Header/CameraComponent.h"
#include "Math/Header/Geometric.h"

CameraComponent::CameraComponent() 
	: m_Right({ 1.0f, 0.0f, 0.0f }), m_Up({ 0.0f, 1.0f, 0.0f }), m_Look({ 0.0f, 0.0f, 1.0f }),
	m_NearZ(0.0f), m_FarZ(0.0f), m_Aspect(0.0f), m_FovY(0.0f), m_NearWindowHeight(0.0f), m_FarWindowHeight(0.0f)

{
}

CameraComponent::~CameraComponent()
{
}

void CameraComponent::SetCameraRightVector(const Float3& right)
{
	m_Right = right;
}

void CameraComponent::SetCameraUpVector(const Float3& up)
{
	m_Up = up;
}

void CameraComponent::SetCameraLookDirection(const Float3& look)
{
	m_Look = look;
}

void CameraComponent::SetLens(float fovY, float aspect, float nearZ, float farZ)
{
	m_FovY = fovY;
	m_Aspect = aspect;
	m_NearZ = nearZ;
	m_FarZ = farZ;

	m_NearWindowHeight = 2.0f * m_NearZ * tanf(0.5f * m_FovY);
	m_FarWindowHeight = 2.0f * m_FarZ * tanf(0.5f * m_FovY);
}

void CameraComponent::OnPitch(float angle)
{
	auto r = Matrix4x4_RotationAxis(m_Right, angle);

	m_Up = Vector3TransformNormal(m_Up, r);
	m_Look = Vector3TransformNormal(m_Look, r);
}

void CameraComponent::OnRotationY(float angle)
{
	auto r = Matrix4x4_RotationY(angle);

	m_Right = Vector3TransformNormal(m_Right, r);
	m_Up = Vector3TransformNormal(m_Up, r);
	m_Look = Vector3TransformNormal(m_Look, r);
}
