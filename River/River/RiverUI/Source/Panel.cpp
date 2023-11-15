#include "RiverPch.h"
#include "RiverUI/Header/Panel.h"
#include "RiverUI/Header/Button.h"

Panel::Panel()
{
}

Panel::~Panel()
{
}

void Panel::OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices)
{
	Widget::OnRender(vertices, indices);

	for (size_t i = 0; i < m_Children.size(); i++)
	{
		m_Children[i]->OnRender(vertices, indices);
	}
}

bool Panel::OnMouseButtonDown(const Event& e)
{
	const MouseButtonEvent& mouseButtonEvent = (const MouseButtonEvent&)e;
	if (MouseIsInPanel(mouseButtonEvent.GetMouseX(), mouseButtonEvent.GetMouseY()))
	{
		for (auto& widget : m_MouseButtonDownDetector)
		{
			if (MouseInWidget(widget, mouseButtonEvent.GetMouseX(), mouseButtonEvent.GetMouseY()))
			{
				if (((Button*)widget.get())->OnMouseButtonDown(e))
				{
					return true;
				}
				
			}
		}
	}
	return false;
}

bool Panel::OnMouseButtonRelease(const Event& e)
{
	return false;
}

bool Panel::MouseIsInPanel(int x, int y)
{
	return false;
}

bool Panel::MouseInWidget(Share<Widget>& widget, int x, int y)
{
	return false;
}
