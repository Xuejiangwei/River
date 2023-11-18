#include "RiverPch.h"
#include "Texture.h"
#include "Renderer/Header/RHI.h"

#ifdef _WIN32
	#include "Renderer/DX12Renderer/Header/DX12Texture.h"
	#include "Renderer/DX12Renderer/Header/DX12RHI.h"
#endif // _WIN32


Texture::Texture(const String& name, const String& path)
	: m_Name(std::move(name)), m_Path(std::move(path))
{
}

Texture::~Texture()
{
}

void Texture::SetTextureId(uint32 textureId)
{
	m_RendererId = textureId;
}

Texture* Texture::CreateTexture(const char* name, const char* filePath)
{
	Texture* ret = nullptr;
	
	switch (RHI::Get()->GetAPIMode())
	{
	case APIMode::DX12:
	{
		if (name && filePath)
		{
			auto dx12Rhi = dynamic_cast<DX12RHI*>(RHI::Get().get());
			ret = dx12Rhi->CreateTexture(name, filePath);
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

	return ret;
}

Texture* Texture::CreateTexture(const char* name, int width, int height, const uint8* data)
{
	DX12Texture* ret = nullptr;

	switch (RHI::Get()->GetAPIMode())
	{
	case APIMode::DX12:
	{
		if (name)
		{
			auto dx12Rhi = dynamic_cast<DX12RHI*>(RHI::Get().get());
			ret = dx12Rhi->CreateTexture(name, width, height, data);
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

	return ret;
}

Texture* Texture::CreateImmediatelyTexture(const char* name, const char* filePath)
{
	DX12Texture* ret = nullptr;

	switch (RHI::Get()->GetAPIMode())
	{
	case APIMode::DX12:
	{
		if (name)
		{
			auto dx12Rhi = dynamic_cast<DX12RHI*>(RHI::Get().get());
			if (dx12Rhi->m_Textures.find(name) == dx12Rhi->m_Textures.end())
			{
				dx12Rhi->WaitFence();
				dx12Rhi->ResetCmdListAlloc();
				dx12Rhi->AddDescriptor(dx12Rhi->CreateTexture(name, filePath));
				dx12Rhi->ExecuteCmdList(false);
				dx12Rhi->WaitFence();
			}
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

	return ret;
}
