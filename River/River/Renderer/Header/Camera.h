#pragma once

#include "Vector.h"

class Camera
{
public:
	Camera() = default;
	virtual ~Camera() {};

	virtual void SetPosition(float x, float y, float z) = 0;

	virtual void Pitch(float angle) = 0;

	virtual void RotateY(float angle) = 0;
	
	virtual void OnMousePressed(int x, int y) = 0;

	virtual void OnMouseReleased(int x, int y) = 0;

	virtual void OnMouseMoved(int x, int y) = 0;

	virtual void SetLens(float fovY, float aspect, float nearZ, float farZ) = 0;

	virtual void OnUpdate() = 0;
};
