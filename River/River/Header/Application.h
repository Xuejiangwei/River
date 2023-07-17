#pragma once

class Layer;
class Window;


class Application
{
public:
	Application();
	virtual ~Application();

	void Run();

	void AddLayer(Share<Layer> layer);

private:
	bool m_Running;

	Vector<Share<Layer>> m_Layers;
	Unique<Window> m_Window;

	static Application* s_Instance;
};

Application* CreateApplication();