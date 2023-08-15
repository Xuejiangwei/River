#pragma once

#include "IndexBuffer.h"

#include <wrl.h>
#include <d3d12.h>

class DX12IndexBuffer : public IndexBuffer
{
public:
	DX12IndexBuffer(ID3D12Device* device, void* indices, uint32_t count, ShaderDataType indiceDataType);

	DX12IndexBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, void* indices, uint32_t count, ShaderDataType indiceDataType);
	
	virtual ~DX12IndexBuffer() override;

	virtual void UpdateData(void* context, void* cmdList, void* indices, size_t count, uint32_t additionalCount = 0) override;

	const D3D12_INDEX_BUFFER_VIEW& GetView() const { return m_IndexBufferView; }

	UINT GetBufferSize() const { return m_IndexBufferView.SizeInBytes; }

	friend class DX12RHI;
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_IndexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_UploaderBuffer;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
};
