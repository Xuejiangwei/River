#include "RiverPch.h"
#include "Physics/Header/PhyScene.h"
#include "Physics/Header/RigidBody.h"

#include "Object/Header/Object.h"
#include "Component/Header/RigidComponent.h"
#include "Component/Header/BaseMeshComponent.h"

PhyScene::PhyScene()
{
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

	auto meshComponent = object->GetComponent<BaseMeshComponent>();
	auto collisionVolume = meshComponent->GetCollisionVolume();
	if (collisionVolume)
	{
		switch (collisionVolume->Type)
		{
		case ColliderType::Box:
		{
			float radius = collisionVolume->Box->HalfSize.Len();
			auto pos = object->GetPosition();
			BoundingSphere bv(pos, radius);
			AddBoundingVolume(bv, rigidComponent->GetRigidBody());
		}
		break;
		case ColliderType::Plane:
		{
			auto scale = object->GetScale();
			float radius = sqrtf(scale.x * scale.x * 100.0f + scale.z * scale.z * 100.0f);

			auto pos = object->GetPosition();
			BoundingSphere bv(pos, radius);
			AddBoundingVolume(bv, rigidComponent->GetRigidBody());
		}
		break;
		case ColliderType::Sphere:
		{
			float radius = collisionVolume->Sphere->Radius;
			auto pos = object->GetPosition();
			BoundingSphere bv(pos, radius);
			AddBoundingVolume(bv, rigidComponent->GetRigidBody());
		}
		break;
		case ColliderType::None:
		default:
			break;
		}
	}
}

void PhyScene::AddBoundingVolume(const BoundingSphere& volume, RigidBody* rigidBody)
{
	if (m_BVH_Tree)
	{
		m_BVH_Tree->AddNode(rigidBody, volume);
	}
	else
	{
		BVH_Node<RigidBody>* parent = nullptr;
		m_BVH_Tree = MakeUnique<BVH_Tree<RigidBody>>(new BVH_Node(parent, rigidBody, volume));
	}
}
