#include "RiverPch.h"
#include "Renderer/DX12Renderer/Header/DX12DescriptorAllocator.h"
#include "Renderer/DX12Renderer/Header/DX12RHI.h"

#define MAX_DESCRIPTOR_NUM 256

V_Array<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> DX12DescriptorAllocator::s_DescriptorHeapPool;

DX12DescriptorAllocator s_DescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] =
{
	D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
	D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
	D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
	D3D12_DESCRIPTOR_HEAP_TYPE_DSV
};

DX12DescriptorAllocator::DX12DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type)
	: m_Type(type), m_DescriptorSize(0), m_RemainingFreeHandles(0), m_CurrentHeap(nullptr)
{
}

DX12DescriptorAllocator::~DX12DescriptorAllocator()
{
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12DescriptorAllocator::Allocate(uint32 count)
{
    if (m_CurrentHeap == nullptr || m_RemainingFreeHandles < count)
    {
        m_CurrentHeap = NewDescriptorHeap(m_Type);
        m_CurrentHandle = m_CurrentHeap->GetCPUDescriptorHandleForHeapStart();
        m_RemainingFreeHandles = MAX_DESCRIPTOR_NUM;

        if (m_DescriptorSize == 0)
        {
            m_DescriptorSize = ((DX12RHI*)RHI::Get().get())->GetDevice()->GetDescriptorHandleIncrementSize(m_Type);
        }
    }

    D3D12_CPU_DESCRIPTOR_HANDLE ret = m_CurrentHandle;
    m_CurrentHandle.ptr += count * (uint64)m_DescriptorSize;
    m_RemainingFreeHandles -= count;

    return ret;
}

ID3D12DescriptorHeap* DX12DescriptorAllocator::NewDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type)
{
    D3D12_DESCRIPTOR_HEAP_DESC Desc;
    Desc.Type = type;
    Desc.NumDescriptors = MAX_DESCRIPTOR_NUM;
    Desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;// D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    Desc.NodeMask = 1;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pHeap;
    ThrowIfFailed(((DX12RHI*)RHI::Get().get())->GetDevice()->CreateDescriptorHeap(&Desc, IID_PPV_ARGS(&pHeap)));
    s_DescriptorHeapPool.push_back(pHeap);

    return pHeap.Get();
}

void DX12DescriptorAllocator::DestroyAllDescriptorHeap()
{
    s_DescriptorHeapPool.clear();
}
