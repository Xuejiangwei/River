#include "RiverPch.h"
#include "RiverUI/Header/Button.h"

Button::Button()
{
}

Button::~Button()
{
}

void Button::BindClickFunction(ClickCall func)
{
	//m_ClickCall = func;
}

bool Button::OnMouseButtonDown(const Event& e)
{

	return true;
}
