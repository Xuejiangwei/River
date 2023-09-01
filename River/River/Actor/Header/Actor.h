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

	template<typename T>
	T* GetComponent(bool mustHave = true)
	{
		T* component = nullptr;
		for (auto& it : m_Components)
		{
			if (component = dynamic_cast<T*>(it.get()))
			{
				return component;
			}
		}

		if (mustHave)
		{
			assert(component);
		}
		return component;
	}

private:
	V_Array<Unique<Component>> m_Components;
};
