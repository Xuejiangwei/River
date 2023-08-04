#pragma once

#include <wrl.h>
#include <d3d12.h>

#include "Renderer/DX12Renderer/Header/d3dx12.h"

class DX12RootSignature
{
public:
	DX12RootSignature(ID3D12Device* device, CD3DX12_ROOT_SIGNATURE_DESC& desc);

	~DX12RootSignature();

	ID3D12RootSignature* GetRootSignature() { return m_RootSignature.Get(); }

	static std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetStaticSamplers();

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
};
