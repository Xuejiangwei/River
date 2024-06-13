#include "RiverPch.h"
#include "Renderer/Texture/Header/Texture.h"
#include "Renderer/Header/RHI.h"
#include "Renderer/Header/AssetManager.h"

#ifdef _WIN32
	#include "Renderer/DX12Renderer/Header/DX12Texture.h"
	#include "Renderer/DX12Renderer/Header/DX12RHI.h"
#endif // _WIN32


Texture::Texture(const String& name, const String& path, Type type)
	: m_Name(name), m_Path(path), m_Type(type)
{
}

Texture::~Texture()
{
}

void Texture::SetTextureId(uint32 textureId)
{
	m_RendererId = textureId;
}

Texture* Texture::CreateTexture(const String& name, const String& filePath, bool isImmediately)
{
	auto assetManager = AssetManager::Get();
	Texture* texture = assetManager->GetTexture(name);
	if (texture)
	{
		return texture;
	}

	switch (RHI::GetAPIMode())
	{
	case APIMode::DX12:
	{
		if (!name.empty() && !filePath.empty())
		{
#ifdef _WIN32
			auto dx12Rhi = dynamic_cast<DX12RHI*>(RHI::Get().get());

			if (filePath.find(".png") != std::string::npos || filePath.find(".PNG") != std::string::npos)
			{
				V_Array<uint8> data;
				uint32 w, h;
				extern void LoadPNG(const char* path, V_Array<uint8>&data, uint32 & width, uint32 & height);
				LoadPNG(filePath.c_str(), data, w, h);

				auto newTexture = dx12Rhi->CreateTexture(name, data.data(), w, h, isImmediately);
				texture = newTexture.get();

				assetManager->AddCacheTexture(name, newTexture);
			}
			else
			{
				auto newTexture = dx12Rhi->CreateTexture(name, filePath, isImmediately);
				texture = newTexture.get();

				assetManager->AddCacheTexture(name, newTexture);
			}

			
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

Texture* Texture::CreateCubeTexture(const String& name, const String& filePath, bool isImmediately)
{
	auto assetManager = AssetManager::Get();
	Texture* texture = assetManager->GetTexture(name);
	if (texture)
	{
		return texture;
	}

	switch (RHI::GetAPIMode())
	{
	case APIMode::DX12:
	{
		if (!name.empty() && !filePath.empty())
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

Texture* Texture::CreateTexture(const String& name, int width, int height, const uint8* data)
{
	auto assetManager = AssetManager::Get();
	Texture* texture = assetManager->GetTexture(name);
	if (texture)
	{
		return texture;
	}

	switch (RHI::GetAPIMode())
	{
	case APIMode::DX12:
	{
		if (!name.empty())
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

Texture* Texture::CreateTexture(const String& name, int width, int height)
{
	auto assetManager = AssetManager::Get();
	Texture* texture = assetManager->GetTexture(name);
	if (texture)
	{
		return texture;
	}

	switch (RHI::GetAPIMode())
	{
	case APIMode::DX12:
	{
		if (!name.empty())
		{
			auto dx12Rhi = dynamic_cast<DX12RHI*>(RHI::Get().get());
			auto newTexture = dx12Rhi->CreateTexture(name, width, height);
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
