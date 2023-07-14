#pragma once

#include "Window.h"

class WindowsWindow : public Window
{
public:
	WindowsWindow();
	virtual ~WindowsWindow() override;

	virtual void Init(const WindowParam& Param) override;

	virtual void OnUpdate() override;

private:

};
