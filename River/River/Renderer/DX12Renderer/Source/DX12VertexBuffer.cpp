#include "RiverPch.h"
#include "Renderer/DX12Renderer/Header/DX12VertexBuffer.h"
#include "Renderer/DX12Renderer/Header/d3dx12.h"


DX12VertexBuffer::DX12VertexBuffer(ID3D12Device* Device, float* vertices, uint32_t size)
{
	auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto buffer = CD3DX12_RESOURCE_DESC::Buffer(size);
	Device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &buffer, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&m_VertexBuffer));
}

DX12VertexBuffer::~DX12VertexBuffer()
{
}