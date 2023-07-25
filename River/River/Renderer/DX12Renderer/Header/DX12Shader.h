#pragma once

#include "Shader.h"

#include <wrl.h>
#include <d3d12.h>

class DX12Shader : public Shader
{
public:
	DX12Shader(const String& filePath, const D3D_SHADER_MACRO* defines, const char* name, const char* target);
	virtual ~DX12Shader() override;

	friend class DX12PipelineState;

	virtual void* GetShader() override { return m_ShaderByteCode.Get(); }

private:
	Microsoft::WRL::ComPtr<ID3D10Blob> m_ShaderByteCode;
};
