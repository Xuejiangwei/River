#include "RiverPch.h"
#include "GUI/Header/Panel.h"
#include "GUI/Header/Image.h"
#include "GUI/Header/Button.h"

#include "Renderer/Texture/Header/Texture.h"
#include "Renderer/DX12Renderer/Header/DX12RHI.h"

#include "Math/Header/Geometric.h"

Panel::Panel()
{
}

Panel::~Panel()
{
}

void Panel::OnUpdate(float deltaTime)
{
	//Widget::OnUpdate(deltaTime);

	for (size_t i = 0; i < m_Children.size(); i++)
	{
		m_Children[i]->OnUpdate(deltaTime);
	}
}

void Panel::OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices)
{
	Widget::OnRender(vertices, indices);

	for (size_t i = 0; i < m_Children.size(); i++)
	{
		m_Children[i]->OnRender(vertices, indices);
	}
}

bool Panel::OnMouseButtonDown(const MouseButtonPressedEvent& e)
{
	const MouseButtonEvent& mouseButtonEvent = (const MouseButtonEvent&)e;
	if (MouseIsInPanel(mouseButtonEvent.GetMouseX(), mouseButtonEvent.GetMouseY()))
	{
		for (auto widget : m_MouseButtonDownDetector)
		{
			if (MouseInWidget(widget, mouseButtonEvent.GetMouseX(), mouseButtonEvent.GetMouseY()))
			{
				if (widget->OnMouseButtonDown(e))
				{
					return true;
				}
				
			}
		}
	}
	return false;
}

bool Panel::OnMouseButtonRelease(const MouseButtonReleasedEvent& e)
{
	const MouseButtonEvent& mouseButtonEvent = (const MouseButtonEvent&)e;
	if (MouseIsInPanel(mouseButtonEvent.GetMouseX(), mouseButtonEvent.GetMouseY()))
	{
		for (auto widget : m_MouseButtonDownDetector)
		{
			if (MouseInWidget(widget, mouseButtonEvent.GetMouseX(), mouseButtonEvent.GetMouseY()))
			{
				if (widget->OnMouseButtonRelease(e))
				{
					return true;
				}

			}
		}
	}
	return false;
}

Share<Widget> Panel::GetChildWidgetByName(const char* name)
{
	for (auto& widget : m_Children)
	{
		if (widget->GetWidgetName() ==  name)
		{
			return widget;
		}
	}
	return nullptr;
}

void Panel::AddMouseButtonDownDetector(Widget* widget)
{
	auto image = dynamic_cast<Image*>(widget);
	if (image)
	{
		m_MouseButtonDownDetector.push_back(image);
	}
}

bool Panel::MouseIsInPanel(int x, int y)
{
	Float2 pos = GetAbsoluteLeftTopPosition();
	Float2 size = GetSize();
	return InRectangle((float)x, (float)y, pos.x, pos.y, size.x, size.y);
}

bool Panel::MouseInWidget(Widget* widget, int x, int y)
{
	Float2 pos = widget->GetAbsoluteLeftTopPosition();
	Float2 size = widget->GetSize();
	return InRectangle((float)x, (float)y, pos.x, pos.y, size.x, size.y);
}
