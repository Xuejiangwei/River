#include "RiverPch.h"
#include "Physics/Header/PhyScene.h"
#include "Physics/Header/RigidBody.h"

#include "Object/Header/Object.h"
#include "Component/Header/RigidComponent.h"

PhyScene::PhyScene()
{
	m_BVH_Tree = new BVH_Tree<RigidBody>();
}

PhyScene::~PhyScene()
{
}

void PhyScene::BeginFrame()
{

}

void PhyScene::EndFrame()
{
}

void PhyScene::OnUpdate(float deltaTime)
{
}

void PhyScene::AddObject(Object* object)
{
	auto rigidComponent = object->GetComponent<RigidComponent>();
	if (rigidComponent)
	{
		if (rigidComponent->GetRigidBody()) 
		{
			m_ObjectRigids[object] = rigidComponent->GetRigidBody();
		}
	}
}
