#pragma once
#include "Component.h"

class CameraObject;

class CameraComponent : public Component
{
	friend class CameraObject;
public:
	CameraComponent(CameraObject* owner);

	virtual ~CameraComponent() override;

	virtual void Tick(float deltaTime) override;

	const Float3& GetRightVector() const { return m_Right; }

	const Float3& GetUpVector() const { return m_Up; }

	const Float3& GetLookVector() const { return m_Look; }

	const Float3& GetPosition() const;

	void SetCameraRightVector(const Float3& right);

	void SetCameraUpVector(const Float3& up);
	
	void SetCameraLookDirection(const Float3& look);

	void SetLens(float fovY, float aspect, float nearZ, float farZ);

	void OnPitch(float angle);

	void OnRotationY(float angle);

	void MoveForward(float value);

	void MoveRight(float value);

	void MoveUp(float value);

private:
	CameraObject* m_Owner;
	Float3 m_Right;
	Float3 m_Up;
	Float3 m_Look;
	float m_NearZ;
	float m_FarZ;
	float m_Aspect;
	float m_FovY;
	float m_NearWindowHeight;
	float m_FarWindowHeight;
	Matrix4x4 m_ViewMatrix;
	Matrix4x4 m_ProjectMatrix;

	bool m_Dirty;
};