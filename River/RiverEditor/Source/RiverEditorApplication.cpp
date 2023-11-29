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

extern Unique<StaticMesh> TestStaticMesh;

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
	Application::Get().GetGameInstance()->SetGameWorld("");

	//Éú³ÉObject
	auto obj = ProduceObject();
	obj->SetPosition({ 1.0f, 3.0f, 1.0f });
	obj->AddComponent(MakeShare<RenderMeshComponent>());
	obj->GetComponent<RenderMeshComponent>()->SetMeshData(RawPolyhedron::GetRawPlane());

	obj = ProduceObject();
	obj->SetPosition({ 1.0f, 1.0f, 3.0f });
	obj->AddComponent(MakeShare<MeshComponent>());
	obj->GetComponent<MeshComponent>()->SetStaticMesh(TestStaticMesh.get());

	auto mat = Material::CreateMaterial("MyMat");
	auto texture = RHI::Get()->GetTexture("tileDiffuseMap");
	auto normalTexture = RHI::Get()->GetTexture("tileNormalMap");
	mat->InitBaseParam({ 1.0f, 1.0f, 1.0f, 1.0f }, { 0.01f,0.01f,0.01f }, 0.25f, 10, texture, normalTexture);
	obj->GetComponent<MeshComponent>()->SetStaticMeshMaterials({ mat });

	obj = ProduceObject();
	obj->SetPosition({ 1.0f, 3.0f, 3.0f });
	obj->AddComponent(MakeShare<RenderMeshComponent>());
	obj->GetComponent<RenderMeshComponent>()->SetMeshData(RawPolyhedron::GetRawBox_24_Points());
}

Application* CreateApplication()
{
	return new RiverEditorApplication();
}