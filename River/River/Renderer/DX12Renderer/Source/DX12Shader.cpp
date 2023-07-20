#include "RiverPch.h"
#include "Renderer/DX12Renderer/Header/DX12Shader.h"
#include "Renderer/DX12Renderer/Header/DX12Util.h"
#include <d3dcompiler.h>

#include "Utils/Header/StringUtils.h"


DX12Shader::DX12Shader(const String& filePath)
{
#if defined(_DEBUG)
	//调试状态下，打开Shader编译的调试标志，不优化
	UINT nCompileFlags =
		D3DCOMPILE_DEBUG
		| D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT nCompileFlags = 0;
#endif

	auto path = S_2_WS(filePath);
	ThrowIfFailed(D3DCompileFromFile(path.c_str(), nullptr,nullptr, "VS", "vs_5_0", nCompileFlags, 0,
		&m_VertexShaderByteCode, nullptr));
	ThrowIfFailed(D3DCompileFromFile(path.c_str(), nullptr, nullptr, "PS", "ps_5_0", nCompileFlags, 0,
		&m_PixelShaderByteCode, nullptr));
}

DX12Shader::~DX12Shader()
{
}