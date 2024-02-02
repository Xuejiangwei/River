#pragma once
#include "Object.h"
#include "Component/Header/CameraComponent.h"

enum class CameraType
{
	OrthoGraphic,
	Perspective
};

class CameraObject : public Object
{
public:
	CameraObject(CameraType type = CameraType::Perspective);
	
	virtual ~CameraObject() override;
	
	virtual void Tick(float deltaTime);

	CameraType GetCameraType() const { return m_CameraType; }

	void Pitch(float angle);
	
	void RotateY(float angle);
	
	void MoveForward(float value);

	void MoveRight(float value);

	void MoveUp(float value);

	void StartRotate(int x, int y);

	void EndRotate();

	void Rotate(float roll, float pitch, float yaw);
	//void SetLens(float fovY, float aspect, float nearZ, float farZ);

	bool IsStartRotate() const { return m_StartRotate; }

	Int2 GetLastMousePosition() const { return m_LastMousePosition; }

private:
	CameraType m_CameraType;
	Share<CameraComponent> m_CameraComponent;

	Int2 m_LastMousePosition;
	bool m_StartRotate;
};