#pragma once

#include <wrl.h>
#include <d3d12.h>

class DX12DescriptorAllocator
{
public:
	DX12DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type);

	~DX12DescriptorAllocator();

	D3D12_CPU_DESCRIPTOR_HANDLE Allocate(uint32 count);

	static ID3D12DescriptorHeap* NewDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type);

	static void DestroyAllDescriptorHeap();

private:
	static V_Array<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> s_DescriptorHeapPool;

	D3D12_DESCRIPTOR_HEAP_TYPE m_Type;
	uint32 m_DescriptorSize;
	uint32 m_RemainingFreeHandles;
	ID3D12DescriptorHeap* m_CurrentHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE m_CurrentHandle;
};
