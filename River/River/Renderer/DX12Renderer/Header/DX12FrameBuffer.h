#pragma once

#include "FrameBuffer.h"
#include "Renderer/DX12Renderer/Header/DX12Util.h"
#include "Renderer/DX12Renderer/Header/DX12UniformBuffer.h"

#include <d3d12.h>
#include <wrl.h>
#include "DirectXMath.h"

struct Vertex
{
	Vertex() = default;
	Vertex(float x, float y, float z, float nx, float ny, float nz, float u, float v) :
		Pos(x, y, z),
		Normal(nx, ny, nz),
		TexC(u, v) {}

	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexC;
};

struct ObjectUniform
{
	DirectX::XMFLOAT4X4 WorldViewProj = Identity4x4();
	DirectX::XMFLOAT4X4 TexTransform = Identity4x4();
};

struct Light
{
	DirectX::XMFLOAT3 Strength = { 0.5f, 0.5f, 0.5f };
	float FalloffStart = 1.0f;                          // point/spot light only
	DirectX::XMFLOAT3 Direction = { 0.0f, -1.0f, 0.0f };// directional/spot light only
	float FalloffEnd = 10.0f;                           // point/spot light only
	DirectX::XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };  // point/spot light only
	float SpotPower = 64.0f;                            // spot light only
};

#define MaxLights 16
struct PassUniform
{
	DirectX::XMFLOAT4X4 View = Identity4x4();
	DirectX::XMFLOAT4X4 InvView = Identity4x4();
	DirectX::XMFLOAT4X4 Proj = Identity4x4();
	DirectX::XMFLOAT4X4 InvProj = Identity4x4();
	DirectX::XMFLOAT4X4 ViewProj = Identity4x4();
	DirectX::XMFLOAT4X4 InvViewProj = Identity4x4();
	DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
	float cbPerObjectPad1 = 0.0f;
	DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
	DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
	float NearZ = 0.0f;
	float FarZ = 0.0f;
	float TotalTime = 0.0f;
	float DeltaTime = 0.0f;

	DirectX::XMFLOAT4 AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };

	DirectX::XMFLOAT4 FogColor = { 0.7f, 0.7f, 0.7f, 1.0f };
	float gFogStart = 5.0f;
	float gFogRange = 150.0f;
	DirectX::XMFLOAT2 cbPerObjectPad2;

	Light Lights[MaxLights];
};

struct MaterialUniform
{
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float Roughness = 0.25f;
	DirectX::XMFLOAT4X4 MatTransform = Identity4x4();
};

class DX12FrameBuffer : public FrameBuffer
{
public:
	DX12FrameBuffer(ID3D12Device* device, UINT passCount, UINT objectCount, UINT materialCount, int waveIndexCount);
	virtual ~DX12FrameBuffer() override;

	DX12FrameBuffer(const DX12FrameBuffer& rhs) = delete;
	DX12FrameBuffer& operator=(const DX12FrameBuffer& rhs) = delete;

	friend class DX12RHI;
private:
	Unique<DX12UniformBuffer<PassUniform>> m_PassUniform;
	Unique<DX12UniformBuffer<ObjectUniform>> m_ObjectUniform;
	Unique<DX12UniformBuffer<MaterialUniform>> m_MaterialUniform;
	Unique<DX12UniformBuffer<Vertex>> WavesVB;

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAlloc;
	UINT64 m_FenceValue;
};
