#pragma once

#include <wrl.h>
#include "d3dx12.h"

class DX12DescriptorAllocator
{
	friend class DescriptorUtils;

public:
	DX12DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type);

	~DX12DescriptorAllocator();

	static ID3D12DescriptorHeap* GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, int numDescriptors = -1);

	static void DestroyAllDescriptorHeap();

	static D3D12_CPU_DESCRIPTOR_HANDLE Allocate(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32 count);

	static CD3DX12_CPU_DESCRIPTOR_HANDLE CpuOffset(ID3D12DescriptorHeap* heap, int index);

	static CD3DX12_GPU_DESCRIPTOR_HANDLE GpuOffset(ID3D12DescriptorHeap* heap, int index);

private:
	D3D12_CPU_DESCRIPTOR_HANDLE Allocate(uint32 count);

private:
	D3D12_DESCRIPTOR_HEAP_TYPE m_Type;
	uint32 m_DescriptorSize;
	uint32 m_RemainingFreeHandles;
	ID3D12DescriptorHeap* m_CurrentHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE m_CurrentHandle;

	V_Array<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> m_DescriptorHeapPool;
};

class DescriptorUtils
{
public:
	static int GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE type);
};