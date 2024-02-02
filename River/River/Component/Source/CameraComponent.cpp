#include "RiverPch.h"
#include "Component/Header/CameraComponent.h"
#include "Object/Header/Object.h"
#include "Math/Header/Geometric.h"

CameraComponent::CameraComponent(Object* owner)
	: m_Owner(owner), m_Right({ 1.0f, 0.0f, 0.0f }), m_Up({ 0.0f, 1.0f, 0.0f }), m_Look({ 0.0f, 0.0f, 1.0f }), m_Dirty(true),
	m_NearZ(0.0f), m_FarZ(0.0f), m_Aspect(0.0f), m_FovY(0.0f), m_NearWindowHeight(0.0f), m_FarWindowHeight(0.0f)
{
}

CameraComponent::~CameraComponent()
{
}

void CameraComponent::Tick(float deltaTime)
{
	if (m_Dirty)
	{
		Float3 position = GetPosition();

		// Keep camera's axes orthogonal to each other and of unit length.
		auto l = VectorNormalize(m_Look);
		auto u = VectorNormalize(VectorCross(l, m_Right));

		// U, L already ortho-normal, so no need to normalize cross product.
		auto r = VectorCross(u, l);

		// Fill in the view matrix entries.
		float x = -VectorDot(position, r).x;
		float y = -VectorDot(position, u).x;
		float z = -VectorDot(position, l).x;

		m_Right = r;
		m_Up = u;
		m_Look = l;

		m_ViewMatrix(0, 0) = m_Right.x;
		m_ViewMatrix(1, 0) = m_Right.y;
		m_ViewMatrix(2, 0) = m_Right.z;
		m_ViewMatrix(3, 0) = x;

		m_ViewMatrix(0, 1) = m_Up.x;
		m_ViewMatrix(1, 1) = m_Up.y;
		m_ViewMatrix(2, 1) = m_Up.z;
		m_ViewMatrix(3, 1) = y;

		m_ViewMatrix(0, 2) = m_Look.x;
		m_ViewMatrix(1, 2) = m_Look.y;
		m_ViewMatrix(2, 2) = m_Look.z;
		m_ViewMatrix(3, 2) = z;

		m_ViewMatrix(0, 3) = 0.0f;
		m_ViewMatrix(1, 3) = 0.0f;
		m_ViewMatrix(2, 3) = 0.0f;
		m_ViewMatrix(3, 3) = 1.0f;

		m_Dirty = false;
	}
}

const Float3& CameraComponent::GetPosition() const
{
	return m_Owner ? m_Owner->GetTransform().Position : Float3(0, 0, 0);
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

	m_Dirty = true;
}

void CameraComponent::OnRotationY(float angle)
{
	auto r = Matrix4x4_RotationY(angle);

	m_Right = Vector3TransformNormal(m_Right, r);
	m_Up = Vector3TransformNormal(m_Up, r);
	m_Look = Vector3TransformNormal(m_Look, r);
}

void CameraComponent::MoveForward(float value)
{
	m_Owner->SetPosition(VectorMultiplyAdd(Float3(value, value, value), m_Look, GetPosition()));
}

void CameraComponent::MoveRight(float value)
{
	m_Owner->SetPosition(VectorMultiplyAdd(Float3(value, value, value), m_Right, GetPosition()));
}

void CameraComponent::MoveUp(float value)
{
	m_Owner->SetPosition(GetPosition() + Float3(0.0, value, 0.0f));
}
