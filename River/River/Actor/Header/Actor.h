#pragma once

#include "RiverHead.h"

class Component;

class Actor
{
public:
	Actor();

	~Actor();

	virtual void Tick(float deltaTime);

	void AddComponent(Unique<Component>& component);

private:
	V_Array<Unique<Component>> m_Components;
};
