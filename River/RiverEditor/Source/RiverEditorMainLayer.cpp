#include "RiverPch.h"
#include "RiverEditorMainLayer.h"

#include "Application.h"
#include "GameInstance.h"
#include "Input/Header/InputManager.h"

#include "GameInstance.h"
#include "Object/Header/ObjectUtils.h"
#include "Object/Header/LightObject.h"
#include "Object/Header/CameraObject.h"
#include "Component/Header/RenderMeshComponent.h"
#include "Component/Header/MeshComponent.h"
#include "Renderer/Mesh/Header/StaticMesh.h"
#include "Renderer/Header/Material.h"
#include "Renderer/Header/RHI.h"
#include "Renderer/Header/AssetManager.h"

#include "Log/Header/Log.h"

RiverEditorMainLayer::RiverEditorMainLayer()
{
}

RiverEditorMainLayer::~RiverEditorMainLayer()
{
}

void RiverEditorMainLayer::OnInitialize()
{
	Application::Get()->GetGameInstance()->SetGameWorld("");

	//生成Object
	/*obj->SetPosition({1.0f, 3.0f, 1.0f});
	obj->AddComponent(MakeShare<RenderMeshComponent>());
	obj->GetComponent<RenderMeshComponent>()->SetMeshData(RawPolyhedron::GetRawPlane());

	/*obj = ProduceObject();*/

	//相机
	{
		m_MainCamera = ProduceObject<CameraObject>();
		m_MainCamera->SetPosition({ 0.0f, 2.0f, -15.0f });
	}

	{
		V_Array<Float3> lights = { {0.57735f, -0.57735f, 0.57735f}, {-0.57735f, -0.57735f, 0.57735f}, {0.0f, -0.707f, -0.707f} };
		V_Array<Float3> lightStrenght = { { 0.9f, 0.8f, 0.7f }, { 0.4f, 0.4f, 0.4f }, { 0.2f, 0.2f, 0.2f } };
		for (size_t i = 0; i < lights.size(); i++)
		{
			auto obj = ProduceObject<LightObject>(lights[i]);
			obj->SetPosition({ 0.0f, 0.0f, 0.0f });
			obj->GetComponent<LightComponent>()->SetLightStrength(lightStrenght[i]);
		}
	}

	auto shader = AssetManager::Get()->GetShader("opaque");
	{
		auto obj = ProduceObject();
		obj->SetPosition({ 2.0f, 1.0f, 0.0f });
		obj->SetScale({ 2.f,2.f,2.f });
		obj->AddComponent(MakeShare<StaticMeshComponent>());
		obj->GetComponent<StaticMeshComponent>()->SetStaticMesh(AssetManager::Get()->GetStaticMesh("DefaultBox"));
		auto mat = Material::CreateMaterial("MyMat");
		auto texture = AssetManager::Get()->GetTexture("tileDiffuseMap");
		auto normalTexture = AssetManager::Get()->GetTexture("tileNormalMap");
		mat->InitBaseParam(MaterialBlendMode::Opaque, shader, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.01f,0.01f,0.01f }, 0.25f, 10, texture, normalTexture);
		obj->GetComponent<StaticMeshComponent>()->SetStaticMeshMaterials({ mat });
	}

	{
		auto obj = ProduceObject();
		obj->SetPosition({ -2.0f, 1.0f, 0.0f });
		obj->SetScale({ 2.f,2.f,2.f });
		obj->AddComponent(MakeShare<StaticMeshComponent>());
		obj->GetComponent<StaticMeshComponent>()->SetStaticMesh(AssetManager::Get()->GetStaticMesh("DefaultBox"));
		auto mat = Material::CreateMaterial("MyMat1");
		auto texture = AssetManager::Get()->GetTexture("bricksDiffuseMap");
		auto normalTexture = AssetManager::Get()->GetTexture("bricksNormalMap");
		mat->InitBaseParam(MaterialBlendMode::Opaque, shader, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.01f,0.01f,0.01f }, 0.25f, 10, texture, normalTexture);
		obj->GetComponent<StaticMeshComponent>()->SetStaticMeshMaterials({ mat });
	}

	{
		auto obj = ProduceObject();
		obj->SetPosition({ 0.0f, -1.0f, 3.0f });
		obj->SetScale({ 1.0f, 1.0f, 1.0f });
		obj->AddComponent(MakeShare<StaticMeshComponent>());
		obj->GetComponent<StaticMeshComponent>()->SetStaticMesh(AssetManager::Get()->GetStaticMesh("DefaultGrid"));
		auto mat = Material::GetMaterial("MyMat");
		obj->GetComponent<StaticMeshComponent>()->SetStaticMeshMaterials({ mat });
	}

	{
		/*shader = AssetManager::Get()->GetShader("sky");
		auto obj = ProduceObject();
		obj->SetPosition({ 0.0f, 0.0f, 0.0f });
		obj->SetScale({ 100.f,100.f,10.f });
		obj->AddComponent(MakeShare<StaticMeshComponent>());
		obj->GetComponent<StaticMeshComponent>()->SetStaticMesh(AssetManager::Get()->GetStaticMesh("DefaultSphere"));
		auto mat = Material::CreateMaterial("MySkyMat");
		auto texture = AssetManager::Get()->GetTexture("skyCubeMap");
		mat->InitBaseParam(MaterialBlendMode::Opaque, shader, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.1f, 0.1f, 0.1f }, 1.0f, 3, texture, nullptr);
		obj->GetComponent<StaticMeshComponent>()->SetStaticMeshMaterials({ mat });*/
	}
}

void RiverEditorMainLayer::OnAttach()
{
}

void RiverEditorMainLayer::OnDetach()
{
}

void RiverEditorMainLayer::OnUpdate(float deltaTime)
{
	auto application = Application::Get();
	auto inputManager = application->GetInputManager();

	float cameraSpeed = 10.0f * deltaTime;
	if (inputManager->GetKeyState(KeyCode::W) == KeyState::Press)
	{
		m_MainCamera->MoveForward(cameraSpeed);
	}
	else if (inputManager->GetKeyState(KeyCode::S) == KeyState::Press)
	{
		m_MainCamera->MoveForward(-cameraSpeed);
	}

	if (inputManager->GetKeyState(KeyCode::D) == KeyState::Press)
	{
		m_MainCamera->MoveRight(cameraSpeed);
	}
	else if (inputManager->GetKeyState(KeyCode::A) == KeyState::Press)
	{
		m_MainCamera->MoveRight(-cameraSpeed);
	}

	if (inputManager->GetKeyState(KeyCode::E) == KeyState::Press)
	{
		m_MainCamera->MoveUp(cameraSpeed);
	}
	else if (inputManager->GetKeyState(KeyCode::Q) == KeyState::Press)
	{
		m_MainCamera->MoveUp(-cameraSpeed);
	}
}

bool RiverEditorMainLayer::OnEvent(const Event& e)
{
	return false;
}

void RiverEditorMainLayer::OnRender()
{
}

bool RiverEditorMainLayer::OnMousePress(MouseCode mouseCode, Int2 mousePosition)
{

	m_MainCamera->StartRotate(mousePosition.x, mousePosition.y);
	return false;
}

bool RiverEditorMainLayer::OnMouseRelease(MouseCode mouseCode, Int2 mousePosition)
{
	m_MainCamera->EndRotate();
	//显示快捷菜单
	return false;
}

bool RiverEditorMainLayer::OnMouseMove(int x, int y)
{
	if (m_MainCamera->IsStartRotate())
	{
		auto lastPos = m_MainCamera->GetLastMousePosition();
		float dx = DegreeToRadians(0.25f * static_cast<float>(x - lastPos.x));
		float dy = DegreeToRadians(0.25f * static_cast<float>(y - lastPos.y));

		m_MainCamera->Rotate(0.0f, dy, dx);
		m_MainCamera->StartRotate(x, y);
		return true;
	}
	return false;
}

bool RiverEditorMainLayer::OnKeyPress(KeyCode key)
{
	return false;
}

bool RiverEditorMainLayer::OnKeyRelease(KeyCode key)
{
	return false;
}

