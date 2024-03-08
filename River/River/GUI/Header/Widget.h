#pragma once


#include "RiverUI.h"
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
	friend class Panel;
public:
	Widget();

	virtual ~Widget();

	virtual void OnUpdate(float deltaTime);

	virtual void OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices);

	virtual void SetWidgetAlign(WidgetAnchors align);

	virtual void SetSize(float w, float h);

	virtual void SetPosition(float x, float y);

	void SetWidgetName(const char* name) { m_Name = name; }

	WidgetAnchors GetWidgetAlign() const { return m_WidgetAlign; }

	Float2 GetSize() const { return m_Size; }

	Float2 GetPosition() const { return m_Position; }

	Float2 GetAbsoluteLeftTopPosition();

	Float2 GetWindowPosition();

protected:
	WidgetAnchors m_WidgetAlign;
	Float2 m_Alignment;
	Float2 m_Size;
	Float2 m_Position;

	Widget* m_Parent;
	String m_Name;
};

