#pragma once

#include "RiverHead.h"
#include "Renderer/Header/Mesh.h"

class Widget;

class UIRenderItem
{
public:
	UIRenderItem(Widget* owner);

	~UIRenderItem();

	void OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices);

private:
	Widget* m_Owner;
};
