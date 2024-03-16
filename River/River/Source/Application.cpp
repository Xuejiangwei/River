#include "RiverPch.h"
#include "Application.h"
#include "GameInstance.h"
#include "Layer.h"
#include "Window.h"
#include "UILayer.h"

#include "RHI.h"
#include "Camera.h"

#include "Input/Header/InputManager.h"
#include "Renderer/Header/RenderScene.h"
#include "Physics/Header/PhyScene.h"

#include "Haze/include/Haze.h"

#pragma comment(lib, "Haze.lib")

Application* Application::s_Instance = nullptr;

Application::Application()
	: m_Running(true)
{
	s_Instance = this;
	m_InputManager = MakeUnique<InputManager>();
	m_CurrentGameInstance = MakeUnique<GameInstance>();

	m_Window = Window::Create();
	WindowParam param = { 720, 720 };
	m_Window->Init(param);

	m_RenderScene = MakeUnique<RenderScene>();

	RHIInitializeParam rhiParam =
	{
		720, 720, m_Window->GetWindowHandle()
	};
	RHI::SetAPIMode(APIMode::DX12);
	RHI::Get()->Initialize(rhiParam);
	m_RenderScene->Initialize();

	AddLayer(MakeShare<UILayer>());

	
}

Application::~Application()
{
}

void Application::Run()
{
	m_Time.Reset();

	const char* args[4] = { "-m", "F:\\GitHub\\River\\River\\HzCode\\HazeCode.hz", "-d", "debug1" };
	auto hazeVm = HazeMain(4, const_cast<char**>(args));

	while (m_Running)
	{
		m_Time.Tick();

		m_Window->OnUpdate();
		if (!m_Running)
		{
			break;
		}

		for (auto& layer : m_Layers)
		{
			layer->OnUpdate(m_Time.DeltaTime());
		}

		m_CurrentGameInstance->OnUpdate(m_Time);

		//hazeVm->StartFunction(HAZE_TEXT("²âÊÔº¯Êý"));

		RHI::Get()->OnUpdate(m_Time);
		m_RenderScene->Update(m_Time);
		m_RenderScene->Render();

		/*for (auto& layer : m_Layers)
		{
			layer->OnRender();
		}*/
		
		//RHI::Get()->Render();
	}
}

void Application::Close()
{
	m_Running = false;
	RHI::Get()->Exit();
}

void Application::AddLayer(Share<Layer> layer)
{
	m_Layers.push_back(layer);
	layer->OnAttach();

	if (layer->IsUILayer())
	{
		m_RenderScene->AddUILayer(layer);
	}
}

void Application::RemoveLayer(Share<Layer> layer)
{
	for (auto iter = m_Layers.begin(); iter != m_Layers.end(); iter++)
	{
		if (*iter == layer)
		{
			layer->OnDetach();
			m_Layers.erase(iter);
			return;
		}
	}
}

void Application::OnEvent(Event& e)
{
	m_InputManager->OnEvent(e, m_Layers);
	for (auto& layer : m_Layers)
	{
		if (layer->OnEvent(e))
		{
			return;
		}
	}

	EventDispatcher dispatcher(e);
	dispatcher.DispatchDirect<MouseButtonPressedEvent>(
		[this](auto e) -> decltype(auto)
		{
			auto& ce = dynamic_cast<MouseButtonPressedEvent&>(e);

			if (ce.GetMouseButton() == MouseCode::ButtonRight)
			{
				RHI::Get()->Pick(ce.GetMouseX(), ce.GetMouseY());
			}
		});
}
