#pragma once

class Layer;
class Window;

#include "Event.h"
#include "RiverTime.h"

class Application
{
public:
	Application();
	virtual ~Application();

	void Run();

	void Close();

	void AddLayer(Share<Layer> layer);

	void RemoveLayer(Share<Layer> layer);

	void OnEvent(Event& e);

	const Unique<Window>& GetWindow() const { return m_Window; }

	static Application& Get() { return *s_Instance; }

private:
	bool m_Running;

	RiverTime m_Time;
	V_Array<Share<Layer>> m_Layers;
	Unique<Window> m_Window;

	static Application* s_Instance;
};

Application* CreateApplication();