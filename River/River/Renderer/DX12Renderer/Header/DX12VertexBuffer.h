#pragma once

#include "VertexBuffer.h"

#include <wrl.h>
#include <d3d12.h>

class DX12VertexBuffer : public VertexBuffer
{
public:
	DX12VertexBuffer(ID3D12Device* device, void* vertices, uint32_t byteSize, uint32_t elementSize, const V_Array<D3D12_INPUT_ELEMENT_DESC>* layout);

	DX12VertexBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, void* vertices, uint32_t size, uint32_t elementSize, const V_Array<D3D12_INPUT_ELEMENT_DESC>* layout);

	virtual ~DX12VertexBuffer() override;

	virtual void UpdateData(void* context, void* cmdList, void* vertices, size_t elementCount, uint32_t additionalCount = 0) override;

	const D3D12_VERTEX_BUFFER_VIEW& GetView() const { return m_VertexBufferView; }

	UINT GetBufferSize() const { return m_VertexBufferView.SizeInBytes; }

	friend class DX12PipelineState;
	friend class DX12RHI;
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_UploaderBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;

	const V_Array<D3D12_INPUT_ELEMENT_DESC>* m_VertexLayout;
};
