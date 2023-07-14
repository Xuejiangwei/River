#include "RiverPch.h"
#include "Application.h"
#include "Window.h"

Application* Application::s_Instance = nullptr;

Application::Application() 
	: m_Running(true)
{
	s_Instance = this;

	m_Window = Window::Create();
	m_Window->Init({ 720, 720 });
}

Application::~Application()
{

}

void Application::Run()
{
	while (m_Running)
	{
		m_Window->OnUpdate();
	}
}