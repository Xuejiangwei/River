#pragma once

#include "FrameBuffer.h"
#include "Renderer/DX12Renderer/Header/DX12Util.h"

#include <d3d12.h>
#include <wrl.h>
#include "DirectXMath.h"

struct ObjectUnifrom
{
    DirectX::XMFLOAT4 World = {1, 0, 0, 1};
    //DirectX::XMFLOAT4X4 World = Identity4x4();
};

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
};

class DX12UniformBuffer;

class DX12FrameBuffer : public FrameBuffer
{
public:
	DX12FrameBuffer(ID3D12Device* device, UINT passCount, UINT objectCount);
	virtual ~DX12FrameBuffer() override;

    friend class DX12RHI;
private:
    Unique<DX12UniformBuffer> PassCB;
    Unique<DX12UniformBuffer> ObjectCB;
    UINT64 m_FenceValue;
};
