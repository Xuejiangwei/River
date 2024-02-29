#include "RiverPch.h"
#include "Object/Header/Object.h"
#include "Component/Header/RenderMeshComponent.h"
#include "Component/Header/BaseMeshComponent.h"
#include "Component/Header/SkeletalMeshComponent.h"
#include "Component/Header/RigidComponent.h"
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

void Object::SetPosition(const Float3& position)
{
	SetTransform(&position, nullptr, nullptr);
}

void Object::SetRotation(const Float4& rotation)
{
	SetTransform(nullptr, &rotation, nullptr);
}

void Object::SetScale(const Float3& scale)
{
	SetTransform(nullptr, nullptr, &scale);
}

void Object::SetTransform(const Transform& transform)
{
	SetTransform(&transform.Position, &transform.Rotation, &transform.Scale);
}

void Object::SetTransform(Float3* position, Float4* rotation, Float3* scale)
{
	SetTransform(const_cast<const Float3*>(position), rotation, scale);
}

void Object::SetTransform(const Float3* position, const Float4* rotation, const Float3* scale)
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
		else if (GetComponent<BaseMeshComponent>())
		{
			createProxy = true;
		}
		else if (GetComponent<SkeletalMeshComponent>())
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
