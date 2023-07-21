#include "RiverPch.h"
#include "Application.h"
#include "Layer.h"
#include "Window.h"

#include "RHI.h"
#include "Camera.h"

Application* Application::s_Instance = nullptr;

Application::Application() 
	: m_Running(true)
{
	s_Instance = this;

	m_Window = Window::Create();
	WindowParam param = { 720, 720 };
	m_Window->Init(param);

	RHIInitializeParam rhiParam =
	{
		720, 720, m_Window->GetWindowHandle()
	};
	RHI::Get()->Initialize(rhiParam);
}

Application::~Application()
{

}

void Application::Run()
{
	while (m_Running)
	{
		for (auto& layer : m_Layers)
		{
			layer->OnUpdate();
		}

		m_Window->OnUpdate();

		RHI::Get()->OnUpdate();
		RHI::Get()->Render();
	}
}

void Application::Close()
{
	m_Running = false;
}

void Application::AddLayer(Share<Layer> layer)
{
	m_Layers.push_back(layer);
}

void Application::OnEvent(Event& e)
{
	for (auto& layer : m_Layers)
	{
		layer->OnEvent(e);
	}

	EventDispatcher dispatcher(e);
	dispatcher.DispatchDirect<MouseButtonPressedEvent>(
		[this](auto e) -> decltype(auto) 
		{
			auto& ce = dynamic_cast<MouseButtonPressedEvent&>(e);
			RHI::Get()->GetMainCamera()->OnMousePressed(ce.GetMouseX(), ce.GetMouseY());
		});

	dispatcher.DispatchDirect<MouseButtonReleasedEvent>(
		[this](auto e) -> decltype(auto)
		{
			auto& ce = dynamic_cast<MouseButtonReleasedEvent&>(e);
			RHI::Get()->GetMainCamera()->OnMouseReleased(ce.GetMouseX(), ce.GetMouseY());
		});

	dispatcher.DispatchDirect<MouseMovedEvent>(
		[this](auto e) -> decltype(auto)
		{
			auto& ce = dynamic_cast<MouseMovedEvent&>(e);
			RHI::Get()->GetMainCamera()->OnMouseMoved((int)ce.GetX(), (int)ce.GetY());
		});
}
