#include "RiverPch.h"
#include "Application.h"
#include "GameInstance.h"
#include "Layer.h"
#include "RiverMainUiLayer.h"
#include "RiverFile.h"
#include "RHI.h"
#include "Camera.h"

#include "Input/Header/InputManager.h"
#include "Renderer/Header/RenderScene.h"
#include "Physics/Header/PhyScene.h"

#include "GUI/Header/GuiManager.h"

#include "Haze.h"
#include "HazeLib/Header/RiverUiLibrary.h"

Application* Application::s_Instance = nullptr;

Application::Application(const WindowParam& windowParam)
	: m_Running(true)
{
	s_Instance = this;
	m_InputManager = MakeUnique<InputManager>();
	m_CurrentGameInstance = MakeUnique<GameInstance>();

	m_Window = Window::Create();
	m_Window->Init(windowParam);

	m_RenderScene = MakeUnique<RenderScene>();

	RHIInitializeParam rhiParam =
	{
		windowParam.WindowWidth, windowParam.WindowHeight, m_Window->GetWindowHandle()
	};
	RHI::SetAPIMode(APIMode::DX12);
	RHI::Get()->Initialize(rhiParam);
	m_RenderScene->Initialize();

	m_MainUiLayer = MakeShare<RiverMainUiLayer>();
	AddLayer(m_MainUiLayer);

	String hazeMainFile = RiverFile::GetPathAddApplicationPath("HzCode\\HazeCode.hz");
	//String hazeMainFile = RiverFile::GetPathAddRootPath("HzCode\\HazeCode.hz");

	String hazeFiles;
	for (auto& codePath : GuiManager::Get()->m_HazeCodePaths)
	{
		if (!hazeFiles.empty())
		{
			hazeFiles += ( " " + codePath);
		}
		else
		{
			hazeFiles = codePath;
		}
	}
	const char* args[6] = { "-m", hazeMainFile.c_str(), "-d", "debug1", "-f", hazeFiles.c_str() };
	m_HazeVM = HazeMain(_countof(args), const_cast<char**>(args));
	RiverUiLibrary::InitializeLib();
}

Application::~Application()
{
}

void Application::Run()
{
	m_Time.Reset();

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

		m_HazeVM->CallFunction(HAZE_TEXT("Ã¿Ö¡¸üÐÂ"), m_Time.DeltaTime());

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
				//RHI::Get()->Pick(ce.GetMouseX(), ce.GetMouseY());
			}
		});
}
