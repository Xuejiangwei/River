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
		if (!m_Window->PeekProcessMessage())
		{
			continue;
		}

		for (auto& layer : m_Layers)
		{
			layer->OnUpdate();
		}

		RHI::Get()->OnUpdate();

		m_Window->OnUpdate();
	}
}

void Application::AddLayer(Share<Layer> layer)
{
	m_Layers.push_back(layer);
}
