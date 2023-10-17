#pragma once

#include "MathStruct.h"
#include "Renderer/Mesh/Header/Mesh.h"
#include "Renderer/Header/RenderItem.h"

enum class WidgetAnchors : uint8
{
	LeftTop,
	RightTop,
	
	LeftCenter,
	RightCenter,

	Center,

	LeftBottom,
	RightBottom
};

class Widget
{
public:
	Widget();

	virtual ~Widget();

	virtual void OnUpdate(float deltaTime);

	virtual void OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices);

	virtual void SetWidgetAlign(WidgetAnchors align);

	virtual void SetSize(float w, float h);

	virtual void SetPosition(float x, float y);

	WidgetAnchors GetWidgetAlign() const { return m_WidgetAlign; }

	FLOAT_2 GetSize() const { return m_Size; }

	FLOAT_2 GetPosition() const { return m_Position; }

	FLOAT_2 GetAbsoluteLeftTopPosition();

protected:
	WidgetAnchors m_WidgetAlign;
	FLOAT_2 m_Alignment;
	FLOAT_2 m_Size;
	FLOAT_2 m_Position;
};

