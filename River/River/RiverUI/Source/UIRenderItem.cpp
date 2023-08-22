#include "RiverPch.h"
#include "RiverUI/Header/UIRenderItem.h"
#include "RiverUI/Header/Widget.h"

UIRenderItem::UIRenderItem(Widget* owner)
	: m_Owner(owner)
{
	assert(owner);
}

UIRenderItem::~UIRenderItem()
{
}

void UIRenderItem::OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices)
{
	auto pos = m_Owner->GetPosition();
	auto size = m_Owner->GetSize();
	float depth = 0.0f;

	vertices.push_back(UIVertex(pos.x, pos.y - size.y, depth, 0.0f, 1.0f, 255, 0, 0, 255));
	vertices.push_back(UIVertex(pos.x, pos.y, depth, 0.0f, 0.0f));
	vertices.push_back(UIVertex(pos.x + size.x, pos.y, depth, 1.0f, 0.0f));
	vertices.push_back(UIVertex(pos.x + size.x, pos.y - size.y, depth, 1.0f, 1.0f));

	indices.push_back((uint32_t)vertices.size() - 4);
	indices.push_back((uint32_t)vertices.size() - 3);
	indices.push_back((uint32_t)vertices.size() - 2);
	indices.push_back((uint32_t)vertices.size() - 4);
	indices.push_back((uint32_t)vertices.size() - 2);
	indices.push_back((uint32_t)vertices.size() - 1);
}
