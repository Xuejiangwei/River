#pragma once

#include "VertexBuffer.h"

#include <wrl.h>
#include <d3d12.h>

class DX12VertexBuffer : public VertexBuffer
{
public:
	DX12VertexBuffer(ID3D12Device* device, float* vertices, uint32_t byteSize, uint32_t elementSize, const VertexBufferLayout& layout);

	DX12VertexBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, float* vertices, uint32_t size, uint32_t elementSize, const VertexBufferLayout& layout);

	~DX12VertexBuffer();

	void InitLayout();

	friend class DX12PipelineState;
	friend class DX12RHI;
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_UploaderBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;

	V_Array<D3D12_INPUT_ELEMENT_DESC> m_VertexLayout;
};
