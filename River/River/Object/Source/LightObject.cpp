#include "RiverPch.h"
#include "Object/Header/LightObject.h"

LightObject::LightObject()
{
	m_LightComponent = MakeShare<LightComponent>();
	AddComponent(m_LightComponent);
}

LightObject::~LightObject()
{
}

void LightObject::Tick(float deltaTime)
{
	Object::Tick(deltaTime);
}
