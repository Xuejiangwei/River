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
	CameraObject(CameraType type);
	
	virtual ~CameraObject() override;
	
	virtual void Tick(float deltaTime);

	CameraType GetCameraType() const { return m_CameraType; }

	void Pitch(float angle);
	
	void RotateY(float angle);
	
	void OnMousePressed(int x, int y);
	
	void OnMouseReleased(int x, int y);
	
	void OnMouseMoved(int x, int y);
	
	void OnKeyPressed(KeyCode code, float time);
	
	//void SetLens(float fovY, float aspect, float nearZ, float farZ);

private:
	CameraType m_CameraType;
	Share<CameraComponent> m_CameraComponent;

	bool m_Dirty;
};