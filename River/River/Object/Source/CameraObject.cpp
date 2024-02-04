#include "RiverPch.h"
#include "Object/Header/CameraObject.h"
#include "Application.h"
#include "Input/Header/InputManager.h"
#include "Renderer/Header/RenderProxy.h"

CameraObject::CameraObject(CameraType type) : m_CameraType(type), m_StartRotate(false)
{
	m_CameraComponent = MakeShare<CameraComponent>(this);
	m_CameraComponent->SetLens(0.25f * MATH_PI, 1.0f, 1.0f, 1000.0f);
	AddComponent(m_CameraComponent);

	m_RenderProxy = MakeUnique<RenderProxy>(this);
	m_RenderProxy->AddCameraObjectProxy();
}

CameraObject::~CameraObject()
{

}

void CameraObject::Tick(float deltaTime)
{
	Object::Tick(deltaTime);
}

void CameraObject::Pitch(float angle)
{
	m_CameraComponent->OnPitch(angle);
}

void CameraObject::RotateY(float angle)
{
	m_CameraComponent->OnRotationY(angle);
}

void CameraObject::MoveForward(float value)
{
	m_CameraComponent->MoveForward(value);
}

void CameraObject::MoveRight(float value)
{
	m_CameraComponent->MoveRight(value);
}

void CameraObject::MoveUp(float value)
{
	m_CameraComponent->MoveUp(value);
}

void CameraObject::StartRotate(int x, int y)
{
	m_LastMousePosition.x = x;
	m_LastMousePosition.y = y;
	m_StartRotate = true;
}

void CameraObject::EndRotate()
{
	m_StartRotate = false;
}

void CameraObject::Rotate(float roll, float pitch, float yaw)
{
	Pitch(pitch);
	RotateY(yaw);
}

const Matrix4x4& CameraObject::GetViewMatrix() const
{
	return m_CameraComponent->m_ViewMatrix;
}

const Matrix4x4& CameraObject::GetProjectMatrix() const
{
	return m_CameraComponent->m_ProjectMatrix;
}
