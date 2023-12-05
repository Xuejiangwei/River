#include "RiverPch.h"
#include "RHI.h"

#include "Renderer/Font/Header/FontAtlas.h"

#ifdef _WIN32
	#include "Renderer/DX12Renderer/Header/DX12RHI.h"
	#include "Renderer/DX12Renderer/Header/DX12VertexBuffer.h"
#endif // _WIN32

APIMode RHI::s_APIMode = APIMode::DX12;

Unique<RHI> RHI::s_Instance = nullptr;

RHI::RHI()
{
}

RHI::~RHI()
{
}

Unique<RHI>& RHI::Get()
{
	if (!s_Instance)
	{
		switch (s_APIMode)
		{
		case APIMode::DX12:
#ifdef _WIN32
			s_Instance = MakeUnique<DX12RHI>();
#endif // _WIN32
			break;
		case APIMode::Vulkan:
			break;
		default:
			break;
		}
	}

	return s_Instance;
}

FontAtlas* RHI::GetFont(const char* name) const
{
	if (!name)
	{
		name = "default";
	}

	auto iter = m_Fonts.find(name);
	return iter->second.get();
}

void RHI::ClearRenderItem()
{
	m_RenderItems.clear();
}

void RHI::ClearUIRenderItem()
{
	m_UIRenderItems.clear();
}

void RHI::AddRenderItem(RenderItem* renderItem, const char* renderPso)
{
	if (renderPso)
	{
		m_RenderItems[renderPso].push_back(*renderItem);
	}
	else
	{
		m_RenderItems["opaque"].push_back(*renderItem);
	}
}

void RHI::AddUIRenderItem(UIRenderItem& renderItem)
{
	m_UIRenderItems.push_back(renderItem);
}
