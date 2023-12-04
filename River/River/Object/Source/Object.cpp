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
	m_Transform.Position = position;
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
