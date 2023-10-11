#pragma once

#include "RiverHead.h"
#include "MathStruct.h"

class Component;

class Object
{
public:
	Object();

	~Object();

	virtual void Tick(float deltaTime);

	void AddComponent(Share<Component> component);

	void SetPosition(const FLOAT_3& position);

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
	FLOAT_3 m_Position;
};
