#include "RiverPch.h"
#include "Renderer/DX12Renderer/Header/DX12RHI.h"
#include "Renderer/DX12Renderer/Header/DX12CommandPool.h"
#include "Renderer/DX12Renderer/Header/DX12Util.h"


Unique<DX12CommandPool> DX12CommandPool::s_CommandPoolInstance = MakeUnique<DX12CommandPool>();

DX12CommandPool::DX12CommandPool()
{
}

DX12CommandPool::~DX12CommandPool()
{
}

int DX12CommandPool::AllocaCommand()
{
	for (int i = 0; i < s_CommandPoolInstance->m_CommandPool.size(); i++)
	{
		if (!s_CommandPoolInstance->m_CommandPool[i].Using)
		{
			s_CommandPoolInstance->m_CommandPool[i].Using = true;
			return i;
		}
	}

	auto device = static_cast<DX12RHI*>(RHI::Get().get())->GetDevice();
	s_CommandPoolInstance->m_CommandPool.resize(s_CommandPoolInstance->m_CommandPool.size() + 1);
	for (size_t i = 0; i < RHI::GetFrameCount(); i++)
	{
		auto& allocator = s_CommandPoolInstance->m_CommandPool.back().Allocator[i];
		auto& list = s_CommandPoolInstance->m_CommandPool.back().List[i];
		ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(allocator.GetAddressOf())));
		ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(),
			nullptr, IID_PPV_ARGS(list.GetAddressOf())));
	}
	

	return (int)s_CommandPoolInstance->m_CommandPool.size();
}

void DX12CommandPool::RecycleCommand(int id)
{
	if (id< s_CommandPoolInstance->m_CommandPool.size())
	{
		s_CommandPoolInstance->m_CommandPool[id].Using = false;
	}
}

inline Microsoft::WRL::ComPtr<ID3D12CommandAllocator>* DX12CommandPool::GetCommandAllocator(int id)
{
	return s_CommandPoolInstance->m_CommandPool[id].Allocator;
}

inline Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>* DX12CommandPool::GetCommandList(int id)
{
	return s_CommandPoolInstance->m_CommandPool[id].List;
}
