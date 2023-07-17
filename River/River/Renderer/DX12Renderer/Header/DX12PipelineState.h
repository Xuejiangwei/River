#pragma once

#include "PipelineState.h"
#include <vector>
#include "Shader.h"

#include <wrl.h>
#include <d3d12.h>

class DX12PipelineState : public PipelineState
{
public:
	DX12PipelineState(ID3D12Device* device, Share<Shader> shader);
	virtual ~DX12PipelineState() override;

	friend class DX12RHI;

private:
	Share<Shader> m_Shader;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState>	m_PipelineState;
};