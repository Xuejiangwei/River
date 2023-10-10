#pragma once

class Layer;
class Window;
class GameInstance;

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

	virtual void Initialize() = 0;

	const Unique<Window>& GetWindow() const { return m_Window; }

	GameInstance* GetGameInstance() const { return m_CurrentGameInstance.get(); }

	static Application& Get() { return *s_Instance; }

private:
	bool m_Running;

	RiverTime m_Time;
	
	V_Array<Share<Layer>> m_Layers;
	
	Unique<GameInstance> m_CurrentGameInstance;
	Unique<Window> m_Window;


	static Application* s_Instance;
};

Application* CreateApplication();