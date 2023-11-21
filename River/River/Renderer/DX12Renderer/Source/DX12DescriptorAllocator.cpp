#include "RiverPch.h"
#include "Renderer/DX12Renderer/Header/DX12DescriptorAllocator.h"
#include "Renderer/DX12Renderer/Header/DX12RHI.h"

#define MAX_DESCRIPTOR_NUM 256

Unique<DX12DescriptorAllocator> s_DescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] =
{
	nullptr, nullptr, nullptr, nullptr
};

DX12DescriptorAllocator::DX12DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type)
	: m_Type(type), m_DescriptorSize(0), m_RemainingFreeHandles(0), m_CurrentHeap(nullptr)
{
    m_DescriptorSize = ((DX12RHI*)RHI::Get().get())->GetDevice()->GetDescriptorHandleIncrementSize(m_Type);
}

DX12DescriptorAllocator::~DX12DescriptorAllocator()
{
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12DescriptorAllocator::Allocate(uint32 count)
{
    if (m_CurrentHeap == nullptr || m_RemainingFreeHandles < count)
    {
        m_CurrentHeap = GetDescriptorHeap(m_Type);
        m_CurrentHandle = m_CurrentHeap->GetCPUDescriptorHandleForHeapStart();
        m_RemainingFreeHandles = MAX_DESCRIPTOR_NUM;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE ret = m_CurrentHandle;
    m_CurrentHandle.ptr += count * (uint64)m_DescriptorSize;
    m_RemainingFreeHandles -= count;

    return ret;
}

ID3D12DescriptorHeap* DX12DescriptorAllocator::GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, int numDescriptors)
{
    D3D12_DESCRIPTOR_HEAP_DESC Desc;
    Desc.Type = type;
    numDescriptors = numDescriptors <= 0 ? MAX_DESCRIPTOR_NUM : numDescriptors;
    Desc.NumDescriptors = numDescriptors;

    switch (type)
    {
    case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
    case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
    {
        Desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;// D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        Desc.NodeMask = 1;
    }
        break;
    case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
    {
        Desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        Desc.NodeMask = 0;
    }
        break;
    case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
    {
        Desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        Desc.NodeMask = 0;
    }
        break;
    case D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES:
        break;
    default:
        break;
    }

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pHeap;
    ThrowIfFailed(((DX12RHI*)RHI::Get().get())->GetDevice()->CreateDescriptorHeap(&Desc, IID_PPV_ARGS(&pHeap)));
    if (!s_DescriptorAllocators[type])
    {
        s_DescriptorAllocators[type] = MakeUnique<DX12DescriptorAllocator>(type);
        s_DescriptorAllocators[type]->m_CurrentHeap = pHeap.Get();
        s_DescriptorAllocators[type]->m_CurrentHandle = pHeap->GetCPUDescriptorHandleForHeapStart();
        s_DescriptorAllocators[type]->m_RemainingFreeHandles = MAX_DESCRIPTOR_NUM;

    }
    s_DescriptorAllocators[type]->m_DescriptorHeapPool.push_back(pHeap);

    return pHeap.Get();
}

void DX12DescriptorAllocator::DestroyAllDescriptorHeap()
{
    for (auto& it : s_DescriptorAllocators)
    {
        if (it)
        {
            it->m_DescriptorHeapPool.clear();
        }
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12DescriptorAllocator::Allocate(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32 count)
{
    GetDescriptorHeap(type);
    return s_DescriptorAllocators[type]->Allocate(count);
}

CD3DX12_CPU_DESCRIPTOR_HANDLE DX12DescriptorAllocator::CpuOffset(ID3D12DescriptorHeap* heap, int index)
{
    auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heap->GetCPUDescriptorHandleForHeapStart());
    int size = s_DescriptorAllocators[heap->GetDesc().Type]->m_DescriptorSize;
    handle.Offset(index, size);
    return handle;

}

CD3DX12_GPU_DESCRIPTOR_HANDLE DX12DescriptorAllocator::GpuOffset(ID3D12DescriptorHeap* heap, int index)
{
    auto handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(heap->GetGPUDescriptorHandleForHeapStart());
    int size = s_DescriptorAllocators[heap->GetDesc().Type]->m_DescriptorSize;
    handle.Offset(index, size);
    return handle;
}

int DescriptorUtils::GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE type)
{
    if (s_DescriptorAllocators[type])
    {
        return s_DescriptorAllocators[type]->m_DescriptorSize;
    }

    return ((DX12RHI*)RHI::Get().get())->GetDevice()->GetDescriptorHandleIncrementSize(type);
}
