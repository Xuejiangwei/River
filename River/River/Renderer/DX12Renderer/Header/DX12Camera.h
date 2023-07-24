#pragma once

#include "Camera.h"
#include "DX12Util.h"

class DX12Camera : public Camera
{
public:
	DX12Camera(CameraType type);
	virtual ~DX12Camera() override;

	virtual void SetPosition(float x, float y, float z) override;

	virtual void Pitch(float angle) override;

	virtual void RotateY(float angle) override;

	virtual void OnMousePressed(int x, int y) override;

	virtual void OnMouseReleased(int x, int y) override;

	virtual void OnMouseMoved(int x, int y) override;

	virtual void SetLens(float fovY, float aspect, float nearZ, float farZ) override;

	virtual void OnUpdate() override;

	void LookAt(DirectX::FXMVECTOR pos, DirectX::FXMVECTOR target, DirectX::FXMVECTOR worldUp);

	const DirectX::XMFLOAT3& GetPosition() const { return m_Position; }

	const DirectX::XMFLOAT3& GetRight() const { return m_Right; }

	const DirectX::XMFLOAT3& GetUp() const { return m_Up; }

	const DirectX::XMFLOAT3& GetLook() const { return m_Look; }

	DirectX::XMMATRIX GetView() const;

	DirectX::XMMATRIX GetProj() const;

private:
	DirectX::XMFLOAT3 m_Position = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 m_Right = { 1.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 m_Up = { 0.0f, 1.0f, 0.0f };
	DirectX::XMFLOAT3 m_Look = { 0.0f, 0.0f, 1.0f };

	float m_NearZ = 0.0f;
	float m_FarZ = 0.0f;
	float m_Aspect = 0.0f;
	float m_FovY = 0.0f;
	float m_NearWindowHeight = 0.0f;
	float m_FarWindowHeight = 0.0f;

	bool m_ViewDirty = true;

	DirectX::XMFLOAT4X4 m_View = Identity4x4();
	DirectX::XMFLOAT4X4 m_Proj = Identity4x4();

	POINT m_LastMousePos;
};