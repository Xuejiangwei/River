#include "RiverPch.h"
#include "RendererUtil.h"
#include "Renderer/DX12Renderer/Header/d3dx12.h"
#include "Renderer/DX12Renderer/Header/DX12Util.h"
#include "Renderer/DX12Renderer/Header/DX12UniformBuffer.h"

//DX12UniformBuffer::DX12UniformBuffer(ID3D12Device* device, UINT elementSize, UINT count, bool minimumGPUAllocSize)
//    : m_ElementByteSize(elementSize)
//{
//    m_ElementByteSize = elementSize;
//
//    // Constant buffer elements need to be multiples of 256 bytes.
//    // This is because the hardware can only view constant data
//    // at m*256 byte offsets and of n*256 byte lengths.
//    // typedef struct D3D12_CONSTANT_BUFFER_VIEW_DESC {
//    // UINT64 OffsetInBytes; // multiple of 256
//    // UINT   SizeInBytes;   // multiple of 256
//    // } D3D12_CONSTANT_BUFFER_VIEW_DESC;
//    if (minimumGPUAllocSize)
//        m_ElementByteSize = RendererUtil::CalcMinimumGPUAllocSize(elementSize);
//
//    auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
//    auto desc = CD3DX12_RESOURCE_DESC::Buffer(m_ElementByteSize * count);
//    ThrowIfFailed(device->CreateCommittedResource(
//        &heapProp,
//        D3D12_HEAP_FLAG_NONE,
//        &desc,
//        D3D12_RESOURCE_STATE_GENERIC_READ,
//        nullptr,
//        IID_PPV_ARGS(&m_UploadBuffer)));
//
//    ThrowIfFailed(m_UploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_MappedData)));
//}
//
//DX12UniformBuffer::~DX12UniformBuffer()
//{
//    if (m_UploadBuffer != nullptr)
//        m_UploadBuffer->Unmap(0, nullptr);
//
//    m_MappedData = nullptr;
//}