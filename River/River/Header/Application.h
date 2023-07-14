#pragma once

class Window;

class Application
{
public:
	Application();
	virtual ~Application();

	void Run();

private:
	bool m_Running;

	Unique<Window> m_Window;

	static Application* s_Instance;
};

Application* CreateApplication();