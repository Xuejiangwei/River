#include "RiverPch.h"
#include "Actor/Header/Actor.h"
#include "Component/Header/Component.h"

Actor::Actor()
{
}

Actor::~Actor()
{
}

void Actor::Tick(float deltaTime)
{
	for (auto& component : m_Components)
	{
		component->Tick(deltaTime);
	}
}

void Actor::AddComponent(Unique<Component>& component)
{
	m_Components.push_back(std::move(component));
}
