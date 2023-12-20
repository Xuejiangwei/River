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
	m_RenderItems.clear();
	m_UIRenderItems.clear();
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

RenderItem* RHI::AddRenderItem()
{
	RenderItem* renderItem = nullptr;
	if (m_UnuseRenderItemId.size() > 0)
	{
		int id = m_UnuseRenderItemId.back();
		m_UnuseRenderItemId.resize(m_UnuseRenderItemId.size() - 1);
		m_RenderItems[id].ObjCBIndex = id;
		renderItem = &m_RenderItems[id];
	}
	else
	{
		if (m_RenderItems.size() < GetRenderItemMaxCount())
		{
			m_RenderItems.resize(m_RenderItems.size() + 1);
			m_RenderItems.back().ObjCBIndex = (int)m_RenderItems.size() - 1;
			renderItem = &m_RenderItems.back();
		}
	}

	return renderItem;
}

void RHI::RemoveRenderItem(int id)
{
	if (id > 0)
	{
		m_RenderItems[id].ObjCBIndex = -1;
		m_UnuseRenderItemId.push_back(id);
	}
}

void RHI::UpdateRenderItem(int id, RenderItem* renderItem)
{
	m_RenderItems[id] = *renderItem;
	m_RenderItems[id].ObjCBIndex = id;
}

void RHI::AddUIRenderItem(UIRenderItem& renderItem)
{
	m_UIRenderItems.push_back(renderItem);
}
