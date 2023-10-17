#pragma once

#include "RiverHead.h"

struct WindowParam
{
	int32_t WindowWidth;
	int32_t WindowHeight;
};

class Window
{
public:
	Window();

	virtual ~Window();

	static Unique<Window> Create();

	virtual void Init(const WindowParam& Param) = 0;

	virtual void OnUpdate() = 0;

	virtual bool PeekProcessMessage() { return false; }

	virtual void* GetWindowHandle() const { return m_WindowHandle; }

	Pair<int, int> GetWindowSize() const { return { m_Param.WindowWidth, m_Param.WindowHeight }; }

protected:
	void* m_WindowHandle;
	WindowParam m_Param;
};