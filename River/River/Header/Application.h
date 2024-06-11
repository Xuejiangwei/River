#pragma once

class InputManager;
class Layer;
class UILayer;
class Window;
class RenderScene;
class PhyScene;
class GameInstance;

class HazeVM;

#include "Event.h"
#include "RiverTime.h"
#include "Window/Header/Window.h"

class Application
{
public:
	Application(const WindowParam& windowParam);

	virtual ~Application();

	void Run();

	void Close();

	void AddLayer(Share<Layer> layer);

	void RemoveLayer(Share<Layer> layer);

	void OnEvent(Event& e);

	virtual void Initialize() = 0;

	const RiverTime& GetTime() const { return m_Time; }

	const Unique<Window>& GetWindow() const { return m_Window; }

	UILayer* GetMainUiLayer() { return m_MainUiLayer.get(); }

	InputManager* GetInputManager() const { return m_InputManager.get(); }

	GameInstance* GetGameInstance() const { return m_CurrentGameInstance.get(); }

	RenderScene* GetRenderScene() const { return m_RenderScene.get(); }

	static Application* Get() { return s_Instance; }

protected:
	bool m_Running;
	RiverTime m_Time;

	Unique<InputManager> m_InputManager;
	Unique<RenderScene> m_RenderScene;
	Unique<PhyScene> m_PhyScene;
	Unique<Window> m_Window;
	Unique<GameInstance> m_CurrentGameInstance;
	Share<UILayer> m_MainUiLayer;
	V_Array<Share<Layer>> m_Layers;

	HazeVM* m_HazeVM;

	static Application* s_Instance;
};

Application* CreateApplication();