#include "RiverPch.h"
#include "RiverEditorApplication.h"
#include "RiverEditorMenuLayer.h"
#include "RiverEditorMainLayer.h"

RiverEditorApplication::RiverEditorApplication()
	: Application()
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