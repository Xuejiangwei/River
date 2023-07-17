#include "RiverPch.h"
#include "Renderer/DX12Renderer/Header/DX12Shader.h"
#include <d3dcompiler.h>

#include "Utils/Header/StringUtils.h"


DX12Shader::DX12Shader(const String& filePath)
{
	auto path = S_2_WS(filePath);
	Microsoft::WRL::ComPtr<ID3DBlob> error1;
	Microsoft::WRL::ComPtr<ID3DBlob> error2;
	D3DCompileFromFile(path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", 0, 0,
		&m_VertexShaderByteCode, &error1);
	D3DCompileFromFile(path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", 0, 0,
		&m_PixelShaderByteCode, &error2);

	assert(error1 == nullptr && error2 == nullptr);
	assert(m_VertexShaderByteCode);
	assert(m_PixelShaderByteCode);

	m_InputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

DX12Shader::~DX12Shader()
{
}