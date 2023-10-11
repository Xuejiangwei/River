#include "RiverPch.h"
#include "Object/Header/Object.h"
#include "Component/Header/Component.h"

Object::Object()
{
}

Object::~Object()
{
}

void Object::Tick(float deltaTime)
{
	for (auto& component : m_Components)
	{
		component->Tick(deltaTime);
	}
}

void Object::SetPosition(const FLOAT_3& position)
{
	m_Position = position;
}

void Object::AddComponent(Share<Component> component)
{
	m_Components.push_back(component);
}
