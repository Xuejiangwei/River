#pragma once

#include "Shader.h"

#include <wrl.h>
#include <d3d12.h>

class DX12Shader : public Shader
{
public:
	DX12Shader(const String& filePath);
	virtual ~DX12Shader() override;

	friend class DX12PipelineState;

	virtual void* GetVertexShader() override { return m_VertexShaderByteCode.Get(); }

	virtual void* GetPixelShader() override { return m_PixelShaderByteCode.Get(); };

private:
	Microsoft::WRL::ComPtr<ID3D10Blob> m_VertexShaderByteCode;
	Microsoft::WRL::ComPtr<ID3D10Blob> m_PixelShaderByteCode;
};
