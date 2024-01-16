#include "RiverPch.h"
#include "Object/Header/LightObject.h"
#include "Renderer/Header/RenderProxy.h"

LightObject::LightObject(Float3 direction)
{
	m_LightComponent = MakeShare<LightComponent>(direction);
	AddComponent(m_LightComponent);
	m_RenderProxy = MakeUnique<RenderProxy>(this);
	m_RenderProxy->AddLightObjectProxy();
}

LightObject::~LightObject()
{
}

void LightObject::Tick(float deltaTime)
{
	Object::Tick(deltaTime);
}
