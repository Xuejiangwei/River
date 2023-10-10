#include "RiverPch.h"
#include "RiverEditorApplication.h"
#include "RiverEditorLayer.h"

#include "Object/Header/ObjectUtils.h"
#include "Component/Header/MeshComponent.h"

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
	auto obj = ProduceObject();
	obj->AddComponent(MakeShare<MeshComponent>());
}

Application* CreateApplication()
{
	return new RiverEditorApplication();
}