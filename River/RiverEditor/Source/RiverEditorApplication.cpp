#include "RiverPch.h"
#include "RiverEditorApplication.h"
#include "RiverEditorLayer.h"

#include "GameInstance.h"
#include "Object/Header/ObjectUtils.h"
#include "Object/Header/LightObject.h"
#include "Component/Header/RenderMeshComponent.h"
#include "Component/Header/MeshComponent.h"
#include "Renderer/Mesh/Header/StaticMesh.h"
#include "Renderer/Header/Material.h"
#include "Renderer/Header/RHI.h"
#include "Renderer/Header/AssetManager.h"

RiverEditorApplication::RiverEditorApplication()
	: Application()
{
	AddLayer(MakeShare<RiverEditorLayer>());
}

RiverEditorApplication::~RiverEditorApplication()
{
}

void RiverEditorApplication::Initialize()
{
	Application::Get()->GetGameInstance()->SetGameWorld("");

	//Éú³ÉObject
	/*obj->SetPosition({1.0f, 3.0f, 1.0f});
	obj->AddComponent(MakeShare<RenderMeshComponent>());
	obj->GetComponent<RenderMeshComponent>()->SetMeshData(RawPolyhedron::GetRawPlane());

	/*obj = ProduceObject();*/
	
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
		obj->SetPosition({ 1.0f, 1.0f, 0.0f });
		obj->SetScale({ 10.f,10.f,10.f });
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
		obj->SetPosition({ -1.0f, 1.0f, 0.0f });
		obj->SetScale({ 10.f,10.f,10.f });
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
		obj->AddComponent(MakeShare<StaticMeshComponent>());
		obj->GetComponent<StaticMeshComponent>()->SetStaticMesh(AssetManager::Get()->GetStaticMesh("DefaultGrid"));
		auto mat = AssetManager::Get()->GetMaterial("MyMat");
		obj->GetComponent<StaticMeshComponent>()->SetStaticMeshMaterials({ mat });
	}

	{
		shader = AssetManager::Get()->GetShader("sky");
		auto obj = ProduceObject();
		obj->SetPosition({ 0.0f, 0.0f, 0.0f });
		obj->SetScale({ 100.f,100.f,100.f });
		obj->AddComponent(MakeShare<StaticMeshComponent>());
		obj->GetComponent<StaticMeshComponent>()->SetStaticMesh(AssetManager::Get()->GetStaticMesh("DefaultSphere"));
		auto mat = Material::CreateMaterial("MySkyMat");
		auto texture = AssetManager::Get()->GetTexture("skyCubeMap");
		mat->InitBaseParam(MaterialBlendMode::Opaque, shader, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.1f, 0.1f, 0.1f }, 1.0f, 3, texture, nullptr);
		obj->GetComponent<StaticMeshComponent>()->SetStaticMeshMaterials({ mat });
	}
}

Application* CreateApplication()
{
	return new RiverEditorApplication();
}