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
	DX12PipelineState(ID3D12Device* device, Share<DX12RootSignature> rootSignature, Share<Shader> vsShader, Share<Shader> psShader, Share<VertexBuffer> vertexBuffer);
	
	virtual ~DX12PipelineState() override;

	friend class DX12RHI;

private:
	Microsoft::WRL::ComPtr<ID3D12PipelineState>	m_PipelineState;
};