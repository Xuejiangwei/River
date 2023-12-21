#include "RiverPch.h"
#include "RiverEditorApplication.h"
#include "RiverEditorLayer.h"

#include "GameInstance.h"
#include "Object/Header/ObjectUtils.h"
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
		auto obj = ProduceObject();
		obj->SetPosition({ 1.0f, 1.0f, 3.0f });
		obj->AddComponent(MakeShare<StaticMeshComponent>());
		obj->GetComponent<StaticMeshComponent>()->SetStaticMesh(MeshAssetManager::Get().GetStaticMesh("DefaultBox"));
		auto mat = Material::CreateMaterial("MyMat");
		auto texture = RHI::Get()->GetTexture("tilediffusemap");
		auto normalTexture = RHI::Get()->GetTexture("tilenormalmap");
		mat->InitBaseParam(MaterialBlendMode::Opaque, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.01f,0.01f,0.01f }, 0.25f, 10, texture, normalTexture);
		obj->GetComponent<StaticMeshComponent>()->SetStaticMeshMaterials({ mat });
	}
	
	

	{
		auto obj = ProduceObject();
		obj->SetPosition({ -1.0f, 1.0f, 3.0f });
		obj->AddComponent(MakeShare<StaticMeshComponent>());
		obj->GetComponent<StaticMeshComponent>()->SetStaticMesh(MeshAssetManager::Get().GetStaticMesh("DefaultBox"));
		auto mat = Material::CreateMaterial("MyMat1");
		auto texture = RHI::Get()->GetTexture("bricksDiffuseMap");
		auto normalTexture = RHI::Get()->GetTexture("bricksNormalMap");
		mat->InitBaseParam(MaterialBlendMode::Opaque, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.01f,0.01f,0.01f }, 0.25f, 10, texture, normalTexture);
		obj->GetComponent<StaticMeshComponent>()->SetStaticMeshMaterials({ mat });
	}

	

	{
		auto obj = ProduceObject();
		obj->SetPosition({ 0.0f, 0.0f, 0.0f });
		obj->SetScale({ 1.f,1.f,1.f });
		obj->AddComponent(MakeShare<StaticMeshComponent>());
		obj->GetComponent<StaticMeshComponent>()->SetStaticMesh(MeshAssetManager::Get().GetStaticMesh("DefaultSphere"));
		auto mat = Material::CreateMaterial("MySkyMat");
		auto texture = RHI::Get()->GetTexture("skyCubeMap");
		mat->InitBaseParam(MaterialBlendMode::Opaque, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.1f, 0.1f, 0.1f }, 1.0f, 3, texture, texture);
		obj->GetComponent<StaticMeshComponent>()->SetStaticMeshMaterials({ mat });
	}
}

Application* CreateApplication()
{
	return new RiverEditorApplication();
}