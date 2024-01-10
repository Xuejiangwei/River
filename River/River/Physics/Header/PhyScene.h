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
	HashMap<Object*, RigidBody*> m_ObjectRigids;
	BVH_Tree<RigidBody>* m_BVH_Tree;
};

