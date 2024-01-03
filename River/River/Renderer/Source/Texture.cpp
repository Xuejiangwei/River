#include "RiverPch.h"
#include "Texture.h"
#include "Renderer/Header/RHI.h"
#include "Renderer/Header/AssetManager.h"

#ifdef _WIN32
	#include "Renderer/DX12Renderer/Header/DX12Texture.h"
	#include "Renderer/DX12Renderer/Header/DX12RHI.h"
#endif // _WIN32


Texture::Texture(const String& name, const String& path, Type type)
	: m_Name(std::move(name)), m_Path(std::move(path)), m_Type(type)
{
}

Texture::~Texture()
{
}

void Texture::SetTextureId(uint32 textureId)
{
	m_RendererId = textureId;
}

Texture* Texture::CreateTexture(const char* name, const char* filePath, bool isImmediately)
{
	auto assetManager = AssetManager::Get();
	Texture* texture = assetManager->GetTexture(name);
	if (texture)
	{
		return texture;
	}

	switch (RHI::Get()->GetAPIMode())
	{
	case APIMode::DX12:
	{
		if (name && filePath)
		{
#ifdef _WIN32
			auto dx12Rhi = dynamic_cast<DX12RHI*>(RHI::Get().get());
			auto newTexture = dx12Rhi->CreateTexture(name, filePath, isImmediately);
			texture = newTexture.get();

			assetManager->AddCacheTexture(name, newTexture);
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

	return texture;
}

Texture* Texture::CreateCubeTexture(const char* name, const char* filePath, bool isImmediately)
{
	auto assetManager = AssetManager::Get();
	Texture* texture = assetManager->GetTexture(name);
	if (texture)
	{
		return texture;
	}

	switch (RHI::Get()->GetAPIMode())
	{
	case APIMode::DX12:
	{
		if (name && filePath)
		{
#ifdef _WIN32
			auto dx12Rhi = dynamic_cast<DX12RHI*>(RHI::Get().get());
			auto newTexture = dx12Rhi->CreateCubeTexture(name, filePath, isImmediately);
			texture = newTexture.get();

			assetManager->AddCacheTexture(name, newTexture);
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

	return texture;
}

Texture* Texture::CreateTexture(const char* name, int width, int height, const uint8* data)
{
	auto assetManager = AssetManager::Get();
	Texture* texture = assetManager->GetTexture(name);
	if (texture)
	{
		return texture;
	}

	switch (RHI::Get()->GetAPIMode())
	{
	case APIMode::DX12:
	{
		if (name)
		{
			auto dx12Rhi = dynamic_cast<DX12RHI*>(RHI::Get().get());
			auto newTexture = dx12Rhi->CreateTexture(name, width, height, data);
			texture = newTexture.get();

			assetManager->AddCacheTexture(name, newTexture);
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

	return texture;
}

Texture* Texture::CreateTextureWithResource(const char* name, void* resoure)
{
	auto assetManager = AssetManager::Get();
	Texture* texture = assetManager->GetTexture(name);
	if (texture)
	{
		return texture;
	}

	switch (RHI::Get()->GetAPIMode())
	{
	case APIMode::DX12:
	{
		if (name)
		{
			auto dx12Rhi = dynamic_cast<DX12RHI*>(RHI::Get().get());
			auto newTexture = dx12Rhi->CreateTextureWithResource(name, resoure);
			texture = newTexture.get();

			assetManager->AddCacheTexture(name, newTexture);
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

	return texture;
}
