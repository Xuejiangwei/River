#include "RiverPch.h"
#include "Renderer/DX12Renderer/Header/DX12DynamicDescriptorHeap.h"
#include "Renderer/DX12Renderer/Header/DX12RHI.h"

#define MAX_DYNAMIC_DESCRIPTOR_NUM 1024

DX12DynamicDescriptorHeap::DX12DynamicDescriptorHeap()
{
    D3D12_DESCRIPTOR_HEAP_DESC Desc;
    Desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    Desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    Desc.NumDescriptors = MAX_DYNAMIC_DESCRIPTOR_NUM;
    Desc.NodeMask = 1;

    ThrowIfFailed(((DX12RHI*)RHI::Get().get())->GetDevice()->CreateDescriptorHeap(&Desc, IID_PPV_ARGS(&m_Heap))); 
}

DX12DynamicDescriptorHeap::~DX12DynamicDescriptorHeap()
{
}
