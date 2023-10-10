#pragma once

#include "RiverHead.h"

class Component;

class Object
{
public:
	Object();

	~Object();

	virtual void Tick(float deltaTime);

	void AddComponent(Share<Component> component);

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
	V_Array<Share<Component>> m_Components;
};
