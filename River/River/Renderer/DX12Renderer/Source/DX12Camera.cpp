#include "RiverPch.h"
#include "Renderer/DX12Renderer/Header/DX12Camera.h"

#include "Application.h"
#include "Window.h"
#include "MathHelper.h"

DX12Camera::DX12Camera(CameraType type)
	: Camera(type)
{
	SetLens(0.25f * PI, 1.0f, 1.0f, 1000.0f);
}

DX12Camera::~DX12Camera()
{
}

void DX12Camera::SetPosition(float x, float y, float z)
{
	m_Position = DirectX::XMFLOAT3(x, y, z);
	m_ViewDirty = true;
}

void DX12Camera::Pitch(float angle)
{
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationAxis(DirectX::XMLoadFloat3(&m_Right), angle);

	DirectX::XMStoreFloat3(&m_Up, XMVector3TransformNormal(DirectX::XMLoadFloat3(&m_Up), R));
	DirectX::XMStoreFloat3(&m_Look, XMVector3TransformNormal(DirectX::XMLoadFloat3(&m_Look), R));

	m_ViewDirty = true;
}

void DX12Camera::RotateY(float angle)
{
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationY(angle);

	DirectX::XMStoreFloat3(&m_Right, XMVector3TransformNormal(DirectX::XMLoadFloat3(&m_Right), R));
	DirectX::XMStoreFloat3(&m_Up, XMVector3TransformNormal(DirectX::XMLoadFloat3(&m_Up), R));
	DirectX::XMStoreFloat3(&m_Look, XMVector3TransformNormal(DirectX::XMLoadFloat3(&m_Look), R));

	m_ViewDirty = true;
}

void DX12Camera::OnMousePressed(int x, int y)
{
	m_LastMousePos.x = x;
	m_LastMousePos.y = y;

	SetCapture((HWND)Application::Get().GetWindow()->GetWindowHandle());
}

void DX12Camera::OnMouseReleased(int x, int y)
{
	ReleaseCapture();
}

void DX12Camera::OnMouseMoved(int x, int y)
{
	if ((void*)GetCapture() == Application::Get().GetWindow()->GetWindowHandle())
	{
		float dx = DirectX::XMConvertToRadians(0.25f * static_cast<float>(x - m_LastMousePos.x));
		float dy = DirectX::XMConvertToRadians(0.25f * static_cast<float>(y - m_LastMousePos.y));

		Pitch(dy);
		RotateY(dx);
	}

	m_LastMousePos.x = x;
	m_LastMousePos.y = y;
}

void DX12Camera::SetLens(float fovY, float aspect, float nearZ, float farZ)
{
	m_FovY = fovY;
	m_Aspect = aspect;
	m_NearZ = nearZ;
	m_FarZ = farZ;

	m_NearWindowHeight = 2.0f * m_NearZ * tanf(0.5f * m_FovY);
	m_FarWindowHeight = 2.0f * m_FarZ * tanf(0.5f * m_FovY);

	DirectX::XMMATRIX P;
	if (m_CameraType == CameraType::Perspective)
	{
		P = DirectX::XMMatrixPerspectiveFovLH(m_FovY, m_Aspect, m_NearZ, m_FarZ);
	}
	else if (m_CameraType == CameraType::OrthoGraphic)
	{
		P = DirectX::XMMatrixOrthographicLH(10, 10, m_NearZ, m_FarZ);
	}
	
	XMStoreFloat4x4(&m_Proj, P);
}

void DX12Camera::OnUpdate()
{
	if (m_ViewDirty)
	{
		DirectX::XMVECTOR R = DirectX::XMLoadFloat3(&m_Right);
		DirectX::XMVECTOR U = DirectX::XMLoadFloat3(&m_Up);
		DirectX::XMVECTOR L = DirectX::XMLoadFloat3(&m_Look);
		DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&m_Position);

		// Keep camera's axes orthogonal to each other and of unit length.
		L = DirectX::XMVector3Normalize(L);
		U = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(L, R));

		// U, L already ortho-normal, so no need to normalize cross product.
		R = DirectX::XMVector3Cross(U, L);

		// Fill in the view matrix entries.
		float x = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(P, R));
		float y = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(P, U));
		float z = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(P, L));

		DirectX::XMStoreFloat3(&m_Right, R);
		DirectX::XMStoreFloat3(&m_Up, U);
		DirectX::XMStoreFloat3(&m_Look, L);

		m_View(0, 0) = m_Right.x;
		m_View(1, 0) = m_Right.y;
		m_View(2, 0) = m_Right.z;
		m_View(3, 0) = x;

		m_View(0, 1) = m_Up.x;
		m_View(1, 1) = m_Up.y;
		m_View(2, 1) = m_Up.z;
		m_View(3, 1) = y;

		m_View(0, 2) = m_Look.x;
		m_View(1, 2) = m_Look.y;
		m_View(2, 2) = m_Look.z;
		m_View(3, 2) = z;

		m_View(0, 3) = 0.0f;
		m_View(1, 3) = 0.0f;
		m_View(2, 3) = 0.0f;
		m_View(3, 3) = 1.0f;

		m_ViewDirty = false;
	}
}

void DX12Camera::OnKeyPressed(KeyCode code, const RiverTime& time)
{
	auto Walk = [this](float d)
	{
		// mPosition += d*mLook
		DirectX::XMVECTOR s = DirectX::XMVectorReplicate(d);
		DirectX::XMVECTOR l = DirectX::XMLoadFloat3(&this->m_Look);
		DirectX::XMVECTOR p = DirectX::XMLoadFloat3(&this->m_Position);
		DirectX::XMStoreFloat3(&this->m_Position, DirectX::XMVectorMultiplyAdd(s, l, p));

		this->m_ViewDirty = true;
	};

	auto Strafe = [this](float d)
	{
		// mPosition += d*mRight
		DirectX::XMVECTOR s = DirectX::XMVectorReplicate(d);
		DirectX::XMVECTOR r = DirectX::XMLoadFloat3(&m_Right);
		DirectX::XMVECTOR p = DirectX::XMLoadFloat3(&m_Position);
		DirectX::XMStoreFloat3(&m_Position, DirectX::XMVectorMultiplyAdd(s, r, p));

		m_ViewDirty = true;
	};	
	float speed = 200.f;
	if (code == KeyCode::W)
	{
		Walk(speed * time.DeltaTime());
	}

	if (code == KeyCode::S)
	{
		Walk(-speed * time.DeltaTime());
	}

	if (code == KeyCode::A)
	{
		Strafe(-speed * time.DeltaTime());
	}

	if (code == KeyCode::D)
	{
		Strafe(speed * time.DeltaTime());
	}

	if (code == KeyCode::Q)
	{
		m_Position.y -= 1.f;
		this->m_ViewDirty = true;
	}

	if (code == KeyCode::E)
	{
		m_Position.y += 1.f;
		this->m_ViewDirty = true;
	}
}

void DX12Camera::OnKeyReleased(KeyCode code, const RiverTime& time)
{
}

void DX12Camera::UpdateViewMatrix()
{

}

void DX12Camera::LookAt(DirectX::FXMVECTOR pos, DirectX::FXMVECTOR target, DirectX::FXMVECTOR worldUp)
{
	DirectX::XMVECTOR L = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(target, pos));
	DirectX::XMVECTOR R = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(worldUp, L));
	DirectX::XMVECTOR U = DirectX::XMVector3Cross(L, R);

	DirectX::XMStoreFloat3(&m_Position, pos);
	DirectX::XMStoreFloat3(&m_Look, L);
	DirectX::XMStoreFloat3(&m_Right, R);
	DirectX::XMStoreFloat3(&m_Up, U);

	m_ViewDirty = true;
}

void DX12Camera::LookAt(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up)
{
	DirectX::XMVECTOR P = XMLoadFloat3(&pos);
	DirectX::XMVECTOR T = XMLoadFloat3(&target);
	DirectX::XMVECTOR U = XMLoadFloat3(&up);

	LookAt(P, T, U);

	m_ViewDirty = true;
}

DirectX::XMMATRIX DX12Camera::GetView() const
{
	return DirectX::XMLoadFloat4x4(&m_View);
}

DirectX::XMMATRIX DX12Camera::GetProj() const
{
	return DirectX::XMLoadFloat4x4(&m_Proj);
}