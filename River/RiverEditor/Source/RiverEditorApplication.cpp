#include "RiverPch.h"
#include "RiverEditorApplication.h"
#include "RiverEditorMenuLayer.h"
#include "RiverEditorMainLayer.h"
#include "../resource.h"

RiverEditorApplication::RiverEditorApplication()
	: Application({ 720, 720, IDI_ICON1, IDI_ICON2 })
{
	AddLayer(MakeShare<RiverEditorMenuLayer>());
	AddLayer(MakeShare<RiverEditorMainLayer>());
}

RiverEditorApplication::~RiverEditorApplication()
{
}

void RiverEditorApplication::Initialize()
{
	for (auto& layer : m_Layers)
	{
		layer->OnInitialize();
	}
}

Application* CreateApplication()
{
	return new RiverEditorApplication();
}