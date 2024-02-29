#pragma once
#include "RiverHead.h"
#include "DataStructure/Header/BVH_Tree.h"

class Object;
class RigidBody;

class PhyScene
{
public:
	PhyScene();

	~PhyScene();

	void BeginFrame();

	void EndFrame();

	void OnUpdate(float deltaTime);
	
	void AddObject(Object* object);

private:
	void AddBoundingVolume(const BoundingSphere& volume, RigidBody* rigidBody);

private:
	HashMap<Object*, RigidBody*> m_ObjectRigids;
	Unique<BVH_Tree<RigidBody>> m_BVH_Tree;
};

