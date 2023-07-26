#pragma once

#include "IndexBuffer.h"

#include <wrl.h>
#include <d3d12.h>

class DX12IndexBuffer : public IndexBuffer
{
public:
	DX12IndexBuffer(ID3D12Device* device, uint32_t* indices, uint32_t count, ShaderDataType indiceDataType);

	DX12IndexBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, uint32_t* indices, uint32_t count, ShaderDataType indiceDataType);
	
	virtual ~DX12IndexBuffer() override;

	friend class DX12RHI;
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_IndexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_UploaderBuffer;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
};
