#pragma once

class InputManager;
class Layer;
class Window;
class RenderScene;
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

	InputManager* GetInputManager() const { return m_InputManager.get(); }

	GameInstance* GetGameInstance() const { return m_CurrentGameInstance.get(); }

	RenderScene* GetRenderScene() const { return m_RenderScene.get(); }

	static Application* Get() { return s_Instance; }

protected:
	bool m_Running;
	RiverTime m_Time;
	
	Unique<InputManager> m_InputManager;
	Unique<RenderScene> m_RenderScene;
	Unique<Window> m_Window;
	Unique<GameInstance> m_CurrentGameInstance;
	V_Array<Share<Layer>> m_Layers;

	static Application* s_Instance;
};

Application* CreateApplication();