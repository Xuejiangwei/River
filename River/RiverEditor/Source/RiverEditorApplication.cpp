#include "RiverPch.h"
#include "RiverEditorApplication.h"
#include "RiverEditorLayer.h"

#include "GameInstance.h"
#include "Object/Header/ObjectUtils.h"
#include "Component/Header/RenderMeshComponent.h"
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
	Application::Get().GetGameInstance()->SetGameWorld("");

	//Éú³ÉObject
	auto obj = ProduceObject();
	obj->SetPosition({ 1.0f, 3.0f, 1.0f });
	obj->AddComponent(MakeShare<RenderMeshComponent>());
	obj->GetComponent<RenderMeshComponent>()->SetMeshData(RawPolyhedron::GetRawPlane());

	obj = ProduceObject();
	obj->SetPosition({ 1.0f, 1.0f, 3.0f });
	obj->AddComponent(MakeShare<RenderMeshComponent>());
	obj->GetComponent<RenderMeshComponent>()->SetMeshData(RawPolyhedron::GetRawPlane());
}

Application* CreateApplication()
{
	return new RiverEditorApplication();
}