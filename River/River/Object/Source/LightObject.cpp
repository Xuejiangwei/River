#include "RiverPch.h"
#include "Object/Header/LightObject.h"

LightObject::LightObject(Float3 direction)
{
	m_LightComponent = MakeShare<LightComponent>(Float3());
	AddComponent(m_LightComponent);
}

LightObject::~LightObject()
{
}

void LightObject::Tick(float deltaTime)
{
	Object::Tick(deltaTime);
}
