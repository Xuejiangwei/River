#pragma once

#include "Shader.h"

#include <wrl.h>
#include <d3d12.h>

class DX12Shader : public Shader
{

public:
	DX12Shader(ID3D12Device* device, const String& filePath, Pair<const D3D_SHADER_MACRO*, const D3D_SHADER_MACRO*>& defines,
		Pair<const char*, const char*>& name, Pair<const char*, const char*> target, V_Array<D3D12_INPUT_ELEMENT_DESC>* layout);

	virtual ~DX12Shader() override;

	friend class DX12PipelineState;

	virtual void* GetVSShader() const override { return m_VSByteCode.Get(); }
	
	virtual void* GetPSShader() const override { return m_PSByteCode.Get(); }

	ID3D12RootSignature* GetRootSignaure() { return m_RootSignature.Get(); }

	ID3D12PipelineState* GetPipelineState() { return m_PipelineState.Get(); }

private:
	Microsoft::WRL::ComPtr<ID3D10Blob> m_VSByteCode;
	Microsoft::WRL::ComPtr<ID3D10Blob> m_PSByteCode;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState>	m_PipelineState;
};