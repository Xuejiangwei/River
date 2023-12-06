#pragma once

#include <wrl.h>
#include "d3dx12.h"

class DX12DynamicDescriptorHeap
{
public:
	DX12DynamicDescriptorHeap();

	~DX12DynamicDescriptorHeap();

	CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuHeapStart()
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_Heap->GetCPUDescriptorHandleForHeapStart());
	}

	CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuHeapStart()
	{
		return CD3DX12_GPU_DESCRIPTOR_HANDLE(m_Heap->GetGPUDescriptorHandleForHeapStart());
	}

	ID3D12DescriptorHeap* GetHeap() { return m_Heap.Get(); }

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_Heap;
};
