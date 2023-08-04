#include "RiverPch.h"
#include "Renderer/Dx12Renderer/Header/DX12IndexBuffer.h"
#include "Renderer/DX12Renderer/Header/d3dx12.h"
#include "Renderer/DX12Renderer/Header/DX12Util.h"

DX12IndexBuffer::DX12IndexBuffer(ID3D12Device* device, void* indices, uint32_t count, ShaderDataType indiceDataType)
	: IndexBuffer(count, indiceDataType)
{
	auto indiceDataSize = ShaderDataTypeSize(indiceDataType);

	D3D12_HEAP_PROPERTIES stHeapProp = { D3D12_HEAP_TYPE_UPLOAD };

	D3D12_RESOURCE_DESC stResSesc = {};
	stResSesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	stResSesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	stResSesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	stResSesc.Format = DXGI_FORMAT_UNKNOWN;
	stResSesc.Width = indiceDataSize;
	stResSesc.Height = 1;
	stResSesc.DepthOrArraySize = 1;
	stResSesc.MipLevels = 1;
	stResSesc.SampleDesc.Count = 1;
	stResSesc.SampleDesc.Quality = 0;

	ThrowIfFailed(device->CreateCommittedResource(&stHeapProp, D3D12_HEAP_FLAG_NONE, &stResSesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_IndexBuffer)));

	UINT8* pVertexDataBegin = nullptr;
	D3D12_RANGE stReadRange = { 0, 0 };
	ThrowIfFailed(m_IndexBuffer->Map(0, &stReadRange, reinterpret_cast<void**>(&pVertexDataBegin)));

	memcpy(pVertexDataBegin, indices, count * indiceDataSize);

	m_IndexBuffer->Unmap(0, nullptr);

	m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
	m_IndexBufferView.Format = ShaderDateTypeToDXGIFormat(indiceDataType);
	m_IndexBufferView.SizeInBytes = count * indiceDataSize;
}

DX12IndexBuffer::DX12IndexBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, void* indices, uint32_t count, ShaderDataType indiceDataType)
	: IndexBuffer(count, indiceDataType)
{
	auto indiceDataSize = ShaderDataTypeSize(indiceDataType);

	m_IndexBuffer = CreateDefaultBuffer(device, commandList, indices, count * indiceDataSize, m_UploaderBuffer);

	m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
	m_IndexBufferView.Format = ShaderDateTypeToDXGIFormat(indiceDataType);
	m_IndexBufferView.SizeInBytes = count * indiceDataSize;
}

DX12IndexBuffer::~DX12IndexBuffer()
{
}