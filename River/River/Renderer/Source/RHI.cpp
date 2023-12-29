#include "RiverPch.h"
#include "RHI.h"

#include "Renderer/Font/Header/FontAtlas.h"

#include <limits>

#ifdef _WIN32
	#include "Renderer/DX12Renderer/Header/DX12RHI.h"
	#include "Renderer/DX12Renderer/Header/DX12VertexBuffer.h"
#endif // _WIN32

APIMode RHI::s_APIMode = APIMode::DX12;

Unique<RHI> RHI::s_Instance = nullptr;

RHI::RHI()
{
	m_RenderItemAllocator.Clear();
	m_UIRenderItemAllocator.Clear();
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
	auto id = m_RenderItemAllocator.Alloc();
	auto i = std::numeric_limits<int>::max();
	/*if (id < 0 || id == std::numeric_limits<decltype(m_RenderItemAllocator)::sizeType>)
	{

	}*/
	m_RenderItemAllocator.m_Containor[id].ObjCBIndex = id;
	RenderItem* renderItem = &m_RenderItemAllocator.m_Containor[id];

	return renderItem;
}

void RHI::RemoveRenderItem(int id)
{
	if (id > 0)
	{
		m_RenderItemAllocator.m_Containor[id].ObjCBIndex = -1;
		m_RenderItemAllocator.Recycle(id);
	}
}

void RHI::UpdateRenderItem(int id, RenderItem* renderItem)
{
	/*m_RenderItems[id] = *renderItem;
	m_RenderItems[id].ObjCBIndex = id;*/
}

void RHI::AddUIRenderItem(UIRenderItem& renderItem)
{
	m_UIRenderItems.push_back(renderItem);
}
