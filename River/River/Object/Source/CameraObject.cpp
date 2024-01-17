#include "RiverPch.h"
#include "Object/Header/CameraObject.h"
#include "Application.h"
#include "Input/Header/InputManager.h"

CameraObject::CameraObject(CameraType type) : m_CameraType(type), m_Dirty(false)
{
	m_CameraComponent = MakeShare<CameraComponent>();
	m_CameraComponent->SetLens(0.25f * MATH_PI, 1.0f, 1.0f, 1000.0f);
	AddComponent(m_CameraComponent);
}

CameraObject::~CameraObject()
{

}

void CameraObject::Tick(float deltaTime)
{
}

void CameraObject::Pitch(float angle)
{
	m_CameraComponent->OnPitch(angle);

	m_Dirty = true;
}

void CameraObject::RotateY(float angle)
{
	m_CameraComponent->OnRotationY(angle);
	
	m_Dirty = true;
}

void CameraObject::OnMousePressed(int x, int y)
{

}

void CameraObject::OnMouseReleased(int x, int y)
{
}

void CameraObject::OnMouseMoved(int x, int y)
{
}

void CameraObject::OnKeyPressed(KeyCode code, float time)
{
}
