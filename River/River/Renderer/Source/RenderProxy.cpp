#include "RiverPch.h"
#include "Renderer/Header/RenderProxy.h"
#include "Renderer/Header/RenderScene.h"
#include "Application.h"

RenderProxy::RenderProxy(Object* object)
	: m_RenderObject(object)
{
	Application::Get()->GetRenderScene()->AddObjectProxyToScene(this);
}

RenderProxy::~RenderProxy()
{
	/*if (Application::Get().GetRenderScene())
	{
		Application::Get().GetRenderScene()->RemoveObjectProxyFromScene(this);
	}*/
}