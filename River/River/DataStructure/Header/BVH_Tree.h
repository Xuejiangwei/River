#pragma once
#include "Math/Header/BaseStruct.h"

class BoundingSphere
{
public:
	BoundingSphere(const Float3& center, float radius);

	BoundingSphere(const BoundingSphere& bs1, const BoundingSphere& bs2);

	~BoundingSphere();

private:
	float m_Radius;
	Float3 m_Center;
};

template<typename NodeData>
class BVH_Node
{
	template<typename NodeData>
	friend class BVH_Tree;
public:
	BVH_Node(NodeData* node, const BoundingSphere& volume);

	~BVH_Node();

	bool IsLeaf() const { return m_NodeData; }

private:
	void UpdateBoundingVolume(bool recurse = true);

private:
	NodeData* m_NodeData;
	BoundingSphere m_BoundingVolume;

	BVH_Node<NodeData>* m_Parent;
	BVH_Node<NodeData>* m_Children[2];
};

template<typename NodeData>
class BVH_Tree
{
public:
	BVH_Tree(BVH_Node<NodeData>* node);

	~BVH_Tree();

	void AddNode(BVH_Node<NodeData>* node);

private:
	BVH_Node<NodeData>* m_Root;
};

#include "BVH_Tree.inl"


