#include "RiverPch.h"
#include "Renderer/DX12Renderer/Header/DX12FrameBuffer.h"

DX12FrameBuffer::DX12FrameBuffer(ID3D12Device* device, UINT passCount, UINT objectCount, UINT materialCount)
	: m_FenceValue(0)
{
	ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_CommandAlloc.GetAddressOf())));
	m_PassUniform = MakeUnique<DX12UniformBuffer<PassUniform>>(device, passCount, true);
	m_ObjectUniform = MakeUnique<DX12UniformBuffer<ObjectUniform>>(device, objectCount, true);
	m_MaterialUniform = MakeUnique<DX12UniformBuffer<MaterialUniform>>(device, materialCount, false); //数组取值的话，应设置为false
}

DX12FrameBuffer::~DX12FrameBuffer()
{
}