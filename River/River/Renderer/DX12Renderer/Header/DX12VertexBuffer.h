#pragma once

#include "VertexBuffer.h"

#include <wrl.h>
#include <d3d12.h>

class DX12VertexBuffer : public VertexBuffer
{
public:
	DX12VertexBuffer(ID3D12Device* Device, float* vertices, uint32_t size, uint32_t elementSize);
	~DX12VertexBuffer();

	friend class DX12RHI;
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_VertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
};

