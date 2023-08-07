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

	const D3D12_INDEX_BUFFER_VIEW& GetView() const { return m_IndexBufferView; }

	friend class DX12RHI;
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_IndexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_UploaderBuffer;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
};
