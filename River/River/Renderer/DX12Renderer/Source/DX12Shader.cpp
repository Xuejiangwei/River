#include "RiverPch.h"
#include "Renderer/DX12Renderer/Header/DX12Shader.h"
#include "Renderer/DX12Renderer/Header/DX12Util.h"
#include <d3dcompiler.h>

#include "Utils/Header/StringUtils.h"

DX12Shader::DX12Shader(const String& filePath, const D3D_SHADER_MACRO* defines, const char* name, const char* target)
{
#if defined(_DEBUG)
	//调试状态下，打开Shader编译的调试标志，不优化
	UINT nCompileFlags = D3DCOMPILE_ENABLE_STRICTNESS |
		D3DCOMPILE_DEBUG
		| D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT nCompileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#endif

	auto path = S_2_WS(filePath);
	if (name)
	{
		Microsoft::WRL::ComPtr<ID3DBlob> errors;
		ThrowIfFailed(D3DCompileFromFile(path.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, name, target, nCompileFlags, 0,
			&m_ShaderByteCode, &errors));

		if (errors != nullptr)
		{
			OutputDebugStringA((char*)errors->GetBufferPointer());
		}
	}
}

DX12Shader::~DX12Shader()
{
}

BYTE* GetShaderBufferPointer(const DX12Shader* shader)
{
	if (shader)
	{
		return reinterpret_cast<BYTE*>(((ID3D10Blob*)shader->GetShader())->GetBufferPointer());
	}

	return nullptr;
}

SIZE_T GetShaderBufferSize(const DX12Shader* shader)
{
	if (shader)
	{
		return ((ID3D10Blob*)shader->GetShader())->GetBufferSize();
	}

	return 0;
}
