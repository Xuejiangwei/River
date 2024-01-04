#pragma once

#include "FrameBuffer.h"
#include "Renderer/DX12Renderer/Header/DX12Util.h"
#include "Renderer/DX12Renderer/Header/DX12UniformBuffer.h"
#include "Renderer/Pass/Header/RenderPass.h"
#include <d3d12.h>
#include <wrl.h>
#include "DirectXMath.h"

struct DX12Vertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexC;
	DirectX::XMFLOAT3 TangentU;
};

struct DX12SkinnedVertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexC;
	DirectX::XMFLOAT3 TangentU;
	DirectX::XMFLOAT3 BoneWeights;
	BYTE BoneIndices[4];
};

struct ObjectUniform
{
	DirectX::XMFLOAT4X4 WorldViewProj = Identity4x4();
	DirectX::XMFLOAT4X4 TexTransform = Identity4x4();
	UINT     MaterialIndex;
	UINT     ObjPad0;
	UINT     ObjPad1;
	UINT     ObjPad2;
};

struct SkinnedUniform
{
	DirectX::XMFLOAT4X4 BoneTransforms[96];
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
	//XMFLOAT4X4 WorldViewProj = Identity4x4();
	DirectX::XMFLOAT4X4 View = Identity4x4();
	DirectX::XMFLOAT4X4 InvView = Identity4x4();
	DirectX::XMFLOAT4X4 Proj = Identity4x4();
	DirectX::XMFLOAT4X4 InvProj = Identity4x4();
	DirectX::XMFLOAT4X4 ViewProj = Identity4x4();
	DirectX::XMFLOAT4X4 InvViewProj = Identity4x4();
	DirectX::XMFLOAT4X4 ViewProjTex = Identity4x4();
	DirectX::XMFLOAT4X4 ShadowTransform = Identity4x4();
	DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
	float cbPerObjectPad1 = 0.0f;
	DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
	DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
	float NearZ = 0.0f;
	float FarZ = 0.0f;
	float TotalTime = 0.0f;
	float DeltaTime = 0.0f;

	DirectX::XMFLOAT4 AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };

	Light Lights[MaxLights];

};

struct SsaoUniform
{
	DirectX::XMFLOAT4X4 Proj;
	DirectX::XMFLOAT4X4 InvProj;
	DirectX::XMFLOAT4X4 ProjTex;
	DirectX::XMFLOAT4   OffsetVectors[14];

	// For SsaoBlur.hlsl
	DirectX::XMFLOAT4 BlurWeights[3];

	DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };

	// Coordinates given in view space.
	float OcclusionRadius = 0.5f;
	float OcclusionFadeStart = 0.2f;
	float OcclusionFadeEnd = 2.0f;
	float SurfaceEpsilon = 0.05f;
};

struct MaterialUniform
{
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float Roughness = 0.25f;
	DirectX::XMFLOAT4X4 MatTransform = Identity4x4();

	UINT DiffuseMapIndex = 0;
	UINT NormalMapIndex = 0;
	UINT MaterialPad1;
	UINT MaterialPad2;
};

struct InstanceUniform
{
	DirectX::XMFLOAT4X4 World = Identity4x4();
	DirectX::XMFLOAT4X4 TexTransform = Identity4x4();
	UINT MaterialIndex;
	UINT InstancePad0;
	UINT InstancePad1;
	UINT InstancePad2;
};

struct UIUniform
{
	DirectX::XMFLOAT2 VertexPos;
	DirectX::XMFLOAT2 VertexUV;
	UINT8 Color[4];
};

class DX12FrameBuffer : public FrameBuffer
{
public:
	DX12FrameBuffer(ID3D12Device* device, UINT passCount, UINT objectCount, UINT skinnedObjectCount, UINT materialCount);
	
	virtual ~DX12FrameBuffer() override;

	DX12FrameBuffer(const DX12FrameBuffer& rhs) = delete;

	DX12FrameBuffer& operator=(const DX12FrameBuffer& rhs) = delete;

	friend class DX12RHI;
	friend class Ssao;
	friend class DX12ShadowPass;
private:
	Unique<DX12UniformBuffer<RenderPass::PassUniform>> m_PassUniform;
	Unique<DX12UniformBuffer<ObjectUniform>> m_ObjectUniform;
	Unique<DX12UniformBuffer<MaterialUniform>> m_MaterialUniform;
	Unique<DX12UniformBuffer<SkinnedUniform>> m_SkinnedUniform;
	Unique<DX12UniformBuffer<SsaoUniform>> m_SsaoUniform;

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAlloc;
	UINT64 m_FenceValue;
};
