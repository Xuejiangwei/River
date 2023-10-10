#include "RiverPch.h"
#include "Renderer/Dx12Renderer/Header/DX12IndexBuffer.h"
#include "Renderer/DX12Renderer/Header/d3dx12.h"
#include "Renderer/DX12Renderer/Header/DX12Util.h"

DX12IndexBuffer::DX12IndexBuffer(ID3D12Device* device, void* indices, uint32_t count, ShaderDataType indiceDataType)
	: IndexBuffer(count, indiceDataType)
{
	auto indiceDataSize = ShaderDataTypeSize(indiceDataType);
	auto size = count * indiceDataSize;

	D3D12_HEAP_PROPERTIES heapProp = { D3D12_HEAP_TYPE_UPLOAD };

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.Width = size;
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;

	ThrowIfFailed(device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc, 
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_IndexBuffer)));

	UINT8* dataBegin = nullptr;
	D3D12_RANGE readRange = { 0, 0 };
	ThrowIfFailed(m_IndexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&dataBegin)));
	memcpy(dataBegin, indices, size);
	m_IndexBuffer->Unmap(0, nullptr);

	m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
	m_IndexBufferView.Format = ShaderDateTypeToDXGIFormat(indiceDataType);
	m_IndexBufferView.SizeInBytes = size;
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

void DX12IndexBuffer::UpdateData(void* context, void* cmdList, void* indices, uint32 count, uint32 additionalCount)
{
	m_Count = count;

	auto indiceDataSize = ShaderDataTypeSize(m_IndiceDataType);
	if (GetBufferSize() < count * indiceDataSize)
	{
		auto size = (count + additionalCount) * indiceDataSize;
		auto device = static_cast<ID3D12Device*>(context);
		auto commandList = static_cast<ID3D12GraphicsCommandList*>(cmdList);

		D3D12_HEAP_PROPERTIES stHeapProp = { D3D12_HEAP_TYPE_UPLOAD };

		D3D12_RESOURCE_DESC stResSesc = {};
		stResSesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		stResSesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		stResSesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		stResSesc.Format = DXGI_FORMAT_UNKNOWN;
		stResSesc.Width = size;
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

		m_IndexBuffer->Unmap(0, &stReadRange);

		m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
		m_IndexBufferView.Format = ShaderDateTypeToDXGIFormat(m_IndiceDataType);
		m_IndexBufferView.SizeInBytes = size;
	}
	else
	{
		UINT8* pVertexDataBegin = nullptr;
		D3D12_RANGE stReadRange = { 0, 0 };
		ThrowIfFailed(m_IndexBuffer->Map(0, &stReadRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, indices, count * indiceDataSize);
		m_IndexBuffer->Unmap(0, nullptr);
	}
}
