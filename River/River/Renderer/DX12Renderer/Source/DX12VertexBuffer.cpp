#include "RiverPch.h"
#include "Renderer/DX12Renderer/Header/DX12VertexBuffer.h"
#include "Renderer/DX12Renderer/Header/d3dx12.h"
#include "Renderer/DX12Renderer/Header/DX12Util.h"

DX12VertexBuffer::DX12VertexBuffer(ID3D12Device* device, float* vertices, uint32_t size, uint32_t elementSize, const VertexBufferLayout& layout)
	: VertexBuffer(layout)
{
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
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_VertexBuffer)));

	UINT8* pVertexDataBegin = nullptr;
	D3D12_RANGE stReadRange = { 0, 0 };
	ThrowIfFailed(m_VertexBuffer->Map(0, &stReadRange, reinterpret_cast<void**>(&pVertexDataBegin)));

	memcpy(pVertexDataBegin, vertices, size);

	m_VertexBuffer->Unmap(0, nullptr);

	m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
	m_VertexBufferView.StrideInBytes = elementSize;
	m_VertexBufferView.SizeInBytes = size;

	m_VertexLayout.resize(m_Layout.GetElementCount());
	auto& layoutElements = m_Layout.GetElements();
	for (size_t i = 0; i < m_VertexLayout.size(); i++)
	{
		m_VertexLayout[i].SemanticName = layoutElements[i].Name.c_str();
		m_VertexLayout[i].SemanticIndex = 0;
		m_VertexLayout[i].Format = ShaderDateTypeToDXGIFormat(layoutElements[i].Type);
		m_VertexLayout[i].InputSlot = 0;
		m_VertexLayout[i].AlignedByteOffset = layoutElements[i].Offset;
		m_VertexLayout[i].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		m_VertexLayout[i].InstanceDataStepRate = 0;
	}
}

DX12VertexBuffer::~DX12VertexBuffer()
{
}
