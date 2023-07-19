#include "RiverPch.h"
#include "RendererUtil.h"
#include "Renderer/DX12Renderer/Header/d3dx12.h"
#include "Renderer/DX12Renderer/Header/DX12Util.h"
#include "Renderer/DX12Renderer/Header/DX12UniformBuffer.h"

DX12UniformBuffer::DX12UniformBuffer(ID3D12Device* device, UINT elementSize, UINT count, bool minimumGPUAllocSize)
    : m_ElementByteSize(elementSize)
{
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
    cbvHeapDesc.NumDescriptors = 1;
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    cbvHeapDesc.NodeMask = 0;
    ThrowIfFailed(device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_UniformBufferHeap)));

    if (minimumGPUAllocSize)
        m_ElementByteSize = RendererUtil::CalcMinimumGPUAllocSize(elementSize);

    auto heapDefault = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(m_ElementByteSize * count);
    device->CreateCommittedResource(
        &heapDefault,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_UploadBuffer));

    m_UploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_MappedData));

    D3D12_GPU_VIRTUAL_ADDRESS cbAddress = m_UploadBuffer->GetGPUVirtualAddress();
   
    int boxCBufIndex = 0;
    cbAddress += boxCBufIndex * m_ElementByteSize;

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
    cbvDesc.BufferLocation = cbAddress;
    cbvDesc.SizeInBytes = m_ElementByteSize;

    device->CreateConstantBufferView(&cbvDesc, m_UniformBufferHeap->GetCPUDescriptorHandleForHeapStart());
}

DX12UniformBuffer::~DX12UniformBuffer()
{
    if (m_UploadBuffer != nullptr)
        m_UploadBuffer->Unmap(0, nullptr);

    m_MappedData = nullptr;
}
