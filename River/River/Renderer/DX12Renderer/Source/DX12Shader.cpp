#include "RiverPch.h"
#include "Renderer/DX12Renderer/Header/DX12Shader.h"
#include "Renderer/DX12Renderer/Header/DX12Util.h"
#include <d3dcompiler.h>

#include "Utils/Header/StringUtils.h"

DX12Shader::DX12Shader(const String& filePath, const D3D_SHADER_MACRO* defines, const char* name, const char* target)
{
#if defined(_DEBUG)
	//����״̬�£���Shader����ĵ��Ա�־�����Ż�
	UINT nCompileFlags =
		D3DCOMPILE_DEBUG
		| D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT nCompileFlags = 0;
#endif

	auto path = S_2_WS(filePath);

	if (name)
	{
		ThrowIfFailed(D3DCompileFromFile(path.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, name, target, nCompileFlags, 0,
			&m_ShaderByteCode, nullptr));
	}
}

DX12Shader::~DX12Shader()
{
}