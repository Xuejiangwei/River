#include "RiverPch.h"
#include "Shader.h"
#include "Renderer/Header/RHI.h"
#include "Renderer/Header/AssetManager.h"

#ifdef _WIN32
	#include "Renderer/DX12Renderer/Header/DX12Shader.h"
	#include "Renderer/DX12Renderer/Header/DX12RHI.h"
#endif // _WIN32

Shader::Shader()
	:m_ShaderId(-1)
{
}

Shader::~Shader()
{
}

Shader* Shader::CreateShader(const String& name, const String& path, Pair<const ShaderDefine*, const ShaderDefine*> defines, ShaderParam* param)
{
	auto assetManager = AssetManager::Get();
	Shader* shader = assetManager->GetShader(name);
	if (shader)
	{
		return shader;
	}

	switch (RHI::GetAPIMode())
	{
	case APIMode::DX12:
	{
		if (!name.empty() && !path.empty())
		{
#ifdef _WIN32
			auto dx12Rhi = dynamic_cast<DX12RHI*>(RHI::Get().get());
			auto newShader = dx12Rhi->CreateShader(name, path, defines, param);
			shader = newShader.get();

			assetManager->AddCacheShader(name, newShader);
#endif
		}
	}
	break;
	case APIMode::Vulkan:
	{
	}
	break;
	default:
		break;
	}

	return shader;
}