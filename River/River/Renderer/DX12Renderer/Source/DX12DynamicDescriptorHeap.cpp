#include "RiverPch.h"
#include "Renderer/DX12Renderer/Header/DX12DynamicDescriptorHeap.h"
#include "Renderer/DX12Renderer/Header/DX12RHI.h"

#define MAX_DYNAMIC_DESCRIPTOR_NUM 65536

DX12DynamicDescriptorHeap::DX12DynamicDescriptorHeap()
{
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    heapDesc.NumDescriptors = MAX_DYNAMIC_DESCRIPTOR_NUM;

    ThrowIfFailed(((DX12RHI*)RHI::Get().get())->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_Heap)));

    //因为Nvidia驱动问题，曾导致m_Heap->GetCPUDescriptorHandleForHeapStart() 返回的Ptr为1
}

DX12DynamicDescriptorHeap::~DX12DynamicDescriptorHeap()
{
}
