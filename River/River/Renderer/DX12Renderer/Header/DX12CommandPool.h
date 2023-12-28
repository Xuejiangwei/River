#pragma once
#include "RiverHead.h"

#include <d3d12.h>
#include <wrl.h>

class DX12CommandPool
{
	friend class DX12RHI;
public:
	DX12CommandPool();

	~DX12CommandPool();

private:
	static int AllocaCommand();

	static void RecycleCommand(int id);

	static Microsoft::WRL::ComPtr<ID3D12CommandAllocator>* GetCommandAllocator(int id);

	static Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>* GetCommandList(int id);

	struct CommandItem
	{
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> Allocator[RHI::GetFrameCount()];
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> List[RHI::GetFrameCount()];
		bool Using;
	};

private:
	static Unique<DX12CommandPool> s_CommandPoolInstance;

	V_Array<CommandItem> m_CommandPool;
};
