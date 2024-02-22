#include "RiverPch.h"
#include "Object/Header/SkyBoxObject.h"
#include "Renderer/Header/RenderProxy.h"

SkyBoxObject::SkyBoxObject()
{
	m_SkyBoxComponent = MakeShare<SkyBoxComponent>();
	AddComponent(m_SkyBoxComponent);

	m_RenderProxy = MakeUnique<RenderProxy>(this);
	m_RenderProxy->AddSkyBoxObjectProxy();
}

SkyBoxObject::~SkyBoxObject()
{
}