#pragma once
#include "RiverHead.h"
#include "DataStructure/Header/BVH_Tree.h"

class RigidBody
{
	friend class BVH_Node<RigidBody>;
public:
	RigidBody();

	~RigidBody();

private:
	BVH_Node<RigidBody>* m_BVH_Node;
};
