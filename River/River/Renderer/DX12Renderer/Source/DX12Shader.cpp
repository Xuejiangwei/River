#include "RiverPch.h"
#include "Renderer/DX12Renderer/Header/DX12Shader.h"
#include <d3dcompiler.h>

#include "Utils/Header/StringUtils.h"


DX12Shader::DX12Shader(const String& filePath)
{
	auto path = S_2_WS(filePath);
	D3DCompileFromFile(path.c_str(), nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &m_VertexShader, nullptr);
	D3DCompileFromFile(path.c_str(), nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &m_PixelShader, nullptr);
}

DX12Shader::~DX12Shader()
{
}