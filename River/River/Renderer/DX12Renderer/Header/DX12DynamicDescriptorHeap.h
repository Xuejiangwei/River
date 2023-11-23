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

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_Heap;
};
