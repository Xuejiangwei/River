#pragma once

#include "PipelineState.h"
#include <vector>
#include "Shader.h"

#include <wrl.h>
#include <d3d12.h>

class DX12RootSignature;
class VertexBuffer;

class DX12PipelineState : public PipelineState
{
public:
	DX12PipelineState(ID3D12Device* device, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc);

	virtual ~DX12PipelineState() override;

	ID3D12PipelineState* GetPSO() { return m_PipelineState.Get(); }

	friend class DX12RHI;

private:
	Microsoft::WRL::ComPtr<ID3D12PipelineState>	m_PipelineState;
};