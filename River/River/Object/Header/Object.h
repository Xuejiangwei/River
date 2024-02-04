#pragma once
#include "RiverHead.h"

class Component;
class RenderProxy;

class Object
{
public:
	Object();

	virtual ~Object();

	virtual void Tick(float deltaTime);

	void AddComponent(Share<Component> component);

	const Transform& GetTransform() const { return m_Transform; }

	const Float3& GetPosition() const { return m_Transform.Position; }

	const Float3& GetRotation() const { return m_Transform.Rotation; }
	
	const Float3& GetScale() const { return m_Transform.Scale; }

	void SetPosition(const Float3& position);

	void SetRotation(const Float4& rotation);

	void SetScale(const Float3& scale);

	void SetTransform(const Transform& transform);

	RenderProxy* GetRenderProxy();

private:
	void SetTransform(Float3* position, Float4* rotation, Float3* scale);
	
	void SetTransform(const Float3* position, const Float4* rotation, const Float3* scale);

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

protected:
	V_Array<Share<Component>> m_Components;
	
	Transform m_Transform;

	Unique<RenderProxy> m_RenderProxy;
};
