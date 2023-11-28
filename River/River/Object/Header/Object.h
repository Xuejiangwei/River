#pragma once

#include "RiverHead.h"
#include "MathStruct.h"

class Component;
class RenderProxy;

class Object
{
public:
	Object();

	~Object();

	virtual void Tick(float deltaTime);

	void AddComponent(Share<Component> component);

	const Transform& GetTransform() const { return m_Transform; }

	void SetPosition(const FLOAT_3& position);

	RenderProxy* GetRenderProxy();

public:
	template<typename T>
	T* GetComponent(bool mustHave = false)
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
	
	Transform m_Transform;

	Unique<RenderProxy> m_RenderProxy;
};
