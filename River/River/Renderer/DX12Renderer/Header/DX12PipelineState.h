#pragma once

#include "PipelineState.h"
#include <vector>
#include "Shader.h"

#include <wrl.h>
#include <d3d12.h>

class VertexBuffer;

class DX12PipelineState : public PipelineState
{
public:
	DX12PipelineState(ID3D12Device* device, Share<Shader> shader, Share<VertexBuffer> vertexBuffer);
	virtual ~DX12PipelineState() override;

	friend class DX12RHI;

private:
	void InitRootSignature(ID3D12Device* device);

	void InitPipelineState(ID3D12Device* device, Share<VertexBuffer> vertexBuffer);

private:
	Share<Shader> m_Shader;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState>	m_PipelineState;
};