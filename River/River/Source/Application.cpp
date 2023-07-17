#include "RiverPch.h"
#include "Application.h"
#include "Layer.h"
#include "Window.h"

#include "Renderer/Header/RHI.h"

Application* Application::s_Instance = nullptr;

Application::Application() 
	: m_Running(true)
{
	s_Instance = this;

	m_Window = Window::Create();
	m_Window->Init({ 720, 720 });

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

		RHI::Get()->Render();

		m_Window->OnUpdate();
	}
}

void Application::AddLayer(Share<Layer> layer)
{
	m_Layers.push_back(layer);
}
