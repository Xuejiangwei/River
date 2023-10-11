#pragma once

#include "MathStruct.h"
#include "Renderer/Mesh/Header/Mesh.h"

class UIRenderItem;

class Widget
{
public:
	Widget();

	virtual ~Widget();

	virtual void OnUpdate(float deltaTime);

	virtual void OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices);

	virtual void SetSize(float w, float h);

	virtual void SetPosition(float x, float y);

	River::Float2 GetSize() const { return m_Size; }

	River::Float2 GetPosition() const { return m_Position; }

protected:
	River::Float2 m_Size;
	River::Float2 m_Position;

	Unique<UIRenderItem> m_RenderItem;
};

