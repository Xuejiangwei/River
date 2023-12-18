#include "RiverPch.h"
#include "Object/Header/Object.h"
#include "Component/Header/RenderMeshComponent.h"
#include "Component/Header/MeshComponent.h"
#include "Component/Header/SkinMeshComponent.h"
#include "Renderer/Header/RenderProxy.h"


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
	SetTransform(&position, nullptr, nullptr);
}

void Object::SetRotation(const FLOAT_4& rotation)
{
	SetTransform(nullptr, &rotation, nullptr);
}

void Object::SetScale(const FLOAT_3& scale)
{
	SetTransform(nullptr, nullptr, &scale);
}

void Object::SetTransform(const Transform& transform)
{
	SetTransform(&transform.Position, &transform.Rotation, &transform.Scale);
}

void Object::SetTransform(FLOAT_3* position, FLOAT_4* rotation, FLOAT_3* scale)
{
	SetTransform(const_cast<const FLOAT_3*>(position), rotation, scale);
}

void Object::SetTransform(const FLOAT_3* position, const FLOAT_4* rotation, const FLOAT_3* scale)
{
	bool markDirty = false;
	if (position)
	{
		m_Transform.Position = *position;
		markDirty = true;
	}

	if (rotation)
	{
		m_Transform.Rotation = *rotation;
		markDirty = true;
	}

	if (scale)
	{
		m_Transform.Scale = *scale;
		markDirty = true;
	}

	if (markDirty && m_RenderProxy)
	{
		m_RenderProxy->MarkDirty();
	}
}

RenderProxy* Object::GetRenderProxy()
{
	if (!m_RenderProxy)
	{
		bool createProxy = false;
		if (GetComponent<RenderMeshComponent>())
		{
			createProxy = true;
		}
		else if (GetComponent<StaticMeshComponent>())
		{
			createProxy = true;
		}
		else if (GetComponent<SkinMeshComponent>())
		{
			createProxy = true;
		}

		if (createProxy)
		{
			m_RenderProxy = MakeUnique<RenderProxy>(this);
		}
	}

	return m_RenderProxy.get();
}

void Object::AddComponent(Share<Component> component)
{
	m_Components.push_back(component);
	GetRenderProxy();
}
