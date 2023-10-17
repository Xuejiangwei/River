#pragma once

#include "Window.h"
#include <Windows.h>

#define WINDOW_CLASS_NAME L"RiverWindowClass"

class WindowsWindow : public Window
{
public:
	WindowsWindow();

	virtual ~WindowsWindow() override;

	virtual void Init(const WindowParam& param) override;

	virtual void OnUpdate() override;

	virtual bool PeekProcessMessage() override;

private:
	MSG m_Msg;
};
