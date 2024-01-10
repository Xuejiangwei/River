#pragma once
#include "Math/Header/BaseStruct.h"

class BoundingSphere
{
public:
	BoundingSphere();

	~BoundingSphere();

private:

};

template<typename NodeData>
class BVH_Node
{
	template<typename NodeData>
	friend class BVH_Tree;
public:
	BVH_Node();

	~BVH_Node();

private:
	NodeData* m_NodeData;
	BoundingSphere m_BoundingVolume;
};

template<typename NodeData>
class BVH_Tree
{
public:
	BVH_Tree();

	~BVH_Tree();

private:
	BVH_Node<NodeData> m_Root;
};

#include "BVH_Tree.inl"


