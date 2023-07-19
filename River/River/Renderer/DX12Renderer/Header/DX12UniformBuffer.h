#pragma once

#include "UniformBuffer.h"

#include <wrl.h>
#include <d3d12.h>

class DX12UniformBuffer : public UniformBuffer
{
public:
    DX12UniformBuffer(ID3D12Device* device, UINT elementSize, UINT count, bool minimumGPUAllocSize);
   
    virtual ~DX12UniformBuffer() override;

    friend class DX12RHI;

    DX12UniformBuffer(const DX12UniformBuffer& rhs) = delete;
    DX12UniformBuffer& operator=(const DX12UniformBuffer& rhs) = delete;

    ID3D12Resource* Resource() const { return m_UploadBuffer.Get(); }

    void CopyData(int elementIndex, void* data, UINT size) { memcpy(&m_MappedData[elementIndex * m_ElementByteSize], &data, size); }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> m_UploadBuffer;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_UniformBufferHeap;

	BYTE* m_MappedData;
	UINT m_ElementByteSize;
};



