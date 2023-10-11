#include "RiverPch.h"
#include "RiverEditorApplication.h"
#include "RiverEditorLayer.h"

#include "GameInstance.h"
#include "Object/Header/ObjectUtils.h"
#include "Component/Header/MeshComponent.h"
#include "Renderer/Mesh/Header/StaticMesh.h"

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
	//Éú³ÉObject
	Application::Get().GetGameInstance()->SetGameWorld("");
	auto obj = ProduceObject();
	obj->AddComponent(MakeShare<MeshComponent>());
	obj->GetComponent<MeshComponent>()->SetStaticMesh(StaticMesh::GetBoxMesh());
	obj->SetPosition({ 0.0f, 0.0f, 1.0f });

	obj = ProduceObject();
	obj->AddComponent(MakeShare<MeshComponent>());
	obj->GetComponent<MeshComponent>()->SetStaticMesh(StaticMesh::GetBoxMesh());
	obj->SetPosition({ 0.0f, 0.0f, 3.0f });
}

Application* CreateApplication()
{
	return new RiverEditorApplication();
}