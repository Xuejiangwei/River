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
#include "Component/Header/StaticMeshComponent.h"
#include "Component/Header/SkeletalMeshComponent.h"
#include "Renderer/Mesh/Header/StaticMesh.h"
#include "Renderer/Mesh/Header/SkeletalMesh.h"
#include "Renderer/Header/Material.h"
#include "Renderer/Header/RHI.h"
#include "Renderer/Header/AssetManager.h"

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

		//Common shader中的gShadowMap可能没有shaderResourceView
	}

	/*auto shader = AssetManager::Get()->GetShader("opaque");
	{
		auto obj = ProduceObject();
		obj->SetPosition({ 2.0f, 1.0f, 0.0f });
		obj->SetScale({ 2.f,2.f,2.f });
		auto staticMeshComponent = MakeShare<StaticMeshComponent>();
		obj->AddComponent(staticMeshComponent);
		staticMeshComponent->SetStaticMesh(AssetManager::Get()->GetStaticMesh("DefaultBox"));
		staticMeshComponent->SetCollider(MakeShare<CollisionVolume>(ColliderType::Box, new ColliderBox({ 2.f, 2.f, 2.f })));

		auto mat = Material::CreateMaterial("MyMat");
		auto texture = AssetManager::Get()->GetTexture("tileDiffuseMap");
		auto normalTexture = AssetManager::Get()->GetTexture("tileNormalMap");
		mat->InitBaseParam(MaterialBlendMode::Opaque, shader, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.01f,0.01f,0.01f }, 0.25f, 10, texture, normalTexture);
		staticMeshComponent->SetStaticMeshMaterials({ mat });
	}

	{
		auto obj = ProduceObject();
		obj->SetPosition({ -2.0f, 1.0f, 0.0f });
		obj->SetScale({ 2.f,2.f,2.f });
		auto staticMeshComponent = MakeShare<StaticMeshComponent>();
		obj->AddComponent(staticMeshComponent);
		staticMeshComponent->SetStaticMesh(AssetManager::Get()->GetStaticMesh("DefaultBox"));
		staticMeshComponent->SetCollider(MakeShare<CollisionVolume>(ColliderType::Box, new ColliderBox({ 2.f, 2.f, 2.f })));

		auto mat = Material::CreateMaterial("MyMat1");
		auto texture = AssetManager::Get()->GetTexture("bricksDiffuseMap");
		auto normalTexture = AssetManager::Get()->GetTexture("bricksNormalMap");
		mat->InitBaseParam(MaterialBlendMode::Opaque, shader, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.01f,0.01f,0.01f }, 0.25f, 10, texture, normalTexture);
		staticMeshComponent->SetStaticMeshMaterials({ mat });
	}

	{
		auto obj = ProduceObject();
		obj->SetPosition({ 0.0f, -1.0f, 3.0f });
		obj->SetScale({ 1.0f, 1.0f, 1.0f });
		auto staticMeshComponent = MakeShare<StaticMeshComponent>();
		obj->AddComponent(staticMeshComponent);
		staticMeshComponent->SetCollider(MakeShare<CollisionVolume>(ColliderType::Plane, 
			new ColliderPlane({ 0.f, 1.f, 0.f }, 1)));

		staticMeshComponent->SetStaticMesh(AssetManager::Get()->GetStaticMesh("DefaultGrid"));
		auto mat = Material::GetMaterial("MyMat");
		staticMeshComponent->SetStaticMeshMaterials({ mat });
	}*/

	{
		auto obj = ProduceObject();
		obj->SetPosition({ 0.f, -5.f, 0.f });
		obj->SetScale({ 1.0f, 1.0f, 1.0f });
		auto skeletalMeshComponent = MakeShare<SkeletalMeshComponent>();
		obj->AddComponent(skeletalMeshComponent);
		skeletalMeshComponent->SetCollider(MakeShare<CollisionVolume>(ColliderType::Box,
			new ColliderPlane({ 1.f, 1.f, 1.f }, 1)));
		skeletalMeshComponent->SetSkeletalMesh(AssetManager::Get()->GetSkeletalMesh("human"));
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
	switch (e.GetEventType())
	{
	case EventType::MouseButtonPressed:
		return OnMousePress(e);
	case EventType::MouseButtonReleased:
		return OnMouseRelease(e);
	case EventType::MouseMoved:
		return OnMouseMove(e);
	default:
		break;
	}
	return false;
}

void RiverEditorMainLayer::OnRender()
{
}

bool RiverEditorMainLayer::OnMousePress(const Event& e)
{
	auto& ce = dynamic_cast<const MouseButtonPressedEvent&>(e);
	m_MainCamera->StartRotate(ce.GetMouseX(), ce.GetMouseY());
	return false;
}

bool RiverEditorMainLayer::OnMouseRelease(const Event& e)
{
	m_MainCamera->EndRotate();
	//显示快捷菜单
	return false;
}

bool RiverEditorMainLayer::OnMouseMove(const Event& e)
{
	auto& ce = dynamic_cast<const MouseMovedEvent&>(e);
	auto x = ce.GetMouseX();
	auto y = ce.GetMouseY();
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

bool RiverEditorMainLayer::OnMouseLeave()
{
	m_MainCamera->EndRotate();
	return false;
}

bool RiverEditorMainLayer::OnKeyPress(const Event& e)
{
	return false;
}

bool RiverEditorMainLayer::OnKeyRelease(const Event& e)
{
	return false;
}

