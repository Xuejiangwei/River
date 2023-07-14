#pragma once

#include "Shader.h"

#include <wrl.h>
#include <d3d12.h>

class DX12Shader : public Shader
{
public:
	DX12Shader(const String& filePath);
	virtual ~DX12Shader() override;

	virtual void* GetVertexShader() override { return m_VertexShader.Get(); }

	virtual void* GetPixelShader() override { return m_PixelShader.Get(); };

private:
	Microsoft::WRL::ComPtr<ID3D10Blob> m_VertexShader;
	Microsoft::WRL::ComPtr<ID3D10Blob> m_PixelShader;
};
