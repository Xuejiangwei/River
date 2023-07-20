#include "RiverPch.h"
#include "Renderer/DX12Renderer/Header/DX12FrameBuffer.h"
#include "Renderer/DX12Renderer/Header/DX12UniformBuffer.h"

DX12FrameBuffer::DX12FrameBuffer(ID3D12Device* device, UINT passCount, UINT objectCount)
    : m_FenceValue(0)
{
    //PassCB = MakeUnique<DX12UniformBuffer>(device, (unsigned int)sizeof(PassUniform), passCount, true);
    //ObjectCB = MakeUnique<DX12UniformBuffer>(device, (unsigned int)sizeof(ObjectUnifrom), objectCount, true);
}

DX12FrameBuffer::~DX12FrameBuffer()
{
}