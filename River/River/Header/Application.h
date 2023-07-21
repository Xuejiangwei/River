#pragma once

class Layer;
class Window;

#include "Event.h"

class Application
{
public:
	Application();
	virtual ~Application();

	void Run();

	void Close();

	void AddLayer(Share<Layer> layer);

	void OnEvent(Event& e);

	const Unique<Window>& GetWindow() const { return m_Window; }

	static Application& Get() { return *s_Instance; }

private:
	bool m_Running;

	Vector<Share<Layer>> m_Layers;
	Unique<Window> m_Window;

	static Application* s_Instance;
};

Application* CreateApplication();