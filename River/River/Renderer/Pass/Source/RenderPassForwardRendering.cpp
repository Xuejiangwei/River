#include "RiverPch.h"
#include "Renderer/Pass/Header/RenderPassForwardRendering.h"
#include "Renderer/Header/RHI.h"
#include "Renderer/Header/RenderScene.h"
#include "Application.h"

RenderPassForwardRendering::RenderPassForwardRendering()
{
}

RenderPassForwardRendering::~RenderPassForwardRendering()
{
}

void RenderPassForwardRendering::Render()
{
	auto& rhi = RHI::Get();
	auto renderScene = Application::Get()->GetRenderScene();

	rhi->SetViewPort(720, 720);

	//先渲染非透明队列
	
}
