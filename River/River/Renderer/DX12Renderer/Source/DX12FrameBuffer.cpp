#include "RiverPch.h"
#include "Renderer/DX12Renderer/Header/DX12FrameBuffer.h"

DX12FrameBuffer::DX12FrameBuffer(ID3D12Device* device, UINT passCount, UINT objectCount, UINT skinnedObjectCount, UINT materialCount)
	: m_FenceValue(0)
{
	ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_CommandAlloc.GetAddressOf())));
	
	m_PassUniform = MakeUnique<DX12UniformBuffer<PassUniform>>(device, passCount, true);
	m_SsaoUniform = MakeUnique<DX12UniformBuffer<SsaoUniform>>(device, 1, true);
	m_MaterialUniform = MakeUnique<DX12UniformBuffer<MaterialUniform>>(device, materialCount, false); //数组取值的话，应设置为false
	m_ObjectUniform = MakeUnique<DX12UniformBuffer<ObjectUniform>>(device, objectCount, true);
	m_SkinnedUniform = std::make_unique<DX12UniformBuffer<SkinnedUniform>>(device, skinnedObjectCount, true);
}

DX12FrameBuffer::~DX12FrameBuffer()
{
}

/*void DX12FrameBuffer::UpdateUIUniform(V_Array<UIVertex>& vertices, V_Array<uint32_t> indices)
{
if (fr->VertexBuffer == nullptr || fr->VertexBufferSize < draw_data->TotalVtxCount)
{
	SafeRelease(fr->VertexBuffer);
	fr->VertexBufferSize = draw_data->TotalVtxCount + 5000;
	D3D12_HEAP_PROPERTIES props;
	memset(&props, 0, sizeof(D3D12_HEAP_PROPERTIES));
	props.Type = D3D12_HEAP_TYPE_UPLOAD;
	props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	D3D12_RESOURCE_DESC desc;
	memset(&desc, 0, sizeof(D3D12_RESOURCE_DESC));
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Width = fr->VertexBufferSize * sizeof(ImDrawVert);
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;
	if (bd->pd3dDevice->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&fr->VertexBuffer)) < 0)
		return;
}
if (fr->IndexBuffer == nullptr || fr->IndexBufferSize < draw_data->TotalIdxCount)
{
	SafeRelease(fr->IndexBuffer);
	fr->IndexBufferSize = draw_data->TotalIdxCount + 10000;
	D3D12_HEAP_PROPERTIES props;
	memset(&props, 0, sizeof(D3D12_HEAP_PROPERTIES));
	props.Type = D3D12_HEAP_TYPE_UPLOAD;
	props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	D3D12_RESOURCE_DESC desc;
	memset(&desc, 0, sizeof(D3D12_RESOURCE_DESC));
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Width = fr->IndexBufferSize * sizeof(ImDrawIdx);
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;
	if (bd->pd3dDevice->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&fr->IndexBuffer)) < 0)
		return;
}
}*/
