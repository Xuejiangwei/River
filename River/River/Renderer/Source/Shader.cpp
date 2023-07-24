#include "RiverPch.h"
#include "Shader.h"

#ifdef _WIN32
#include "Renderer/DX12Renderer/Header/DX12Shader.h"
#endif // _WIN32

Shader::Shader()
{
}

Shader::~Shader()
{
}

Share<Shader> Shader::Create(const String& filePath)
{
#ifdef _WIN32
	return MakeShare<DX12Shader>(filePath);
#endif // _WIN32

	return nullptr;
}