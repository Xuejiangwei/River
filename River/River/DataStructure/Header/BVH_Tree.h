#pragma once
#include "Math/Header/BaseStruct.h"
#include "Math/Header/BaseDefine.h"

class BoundingSphere
{
public:
	BoundingSphere(const Float3& center, float radius);

	BoundingSphere(const BoundingSphere& bs1, const BoundingSphere& bs2);

	~BoundingSphere();

	float BoundingSphere::GetVolume() const
	{
		return (4 / 3.f * MATH_PI) * m_Radius * m_Radius * m_Radius;
	}

	bool BoundingSphere::IsOverlapWith(const BoundingSphere& other) const
	{
		return (m_Center - other.m_Center).LenSquared() <= (m_Radius + other.m_Radius) * (m_Radius + other.m_Radius);
	}

	float BoundingSphere::GetGrowth(const BoundingSphere& other) const
	{
		// ֱ�ӹ���һ����Χ����������°�Χ��
		BoundingSphere tmpSphere(*this, other);
		// �ð뾶ƽ������������ʾ��������(ĳЩ�㷨���ñ��������Goldsmith-Salmon�㷨)
		return tmpSphere.m_Radius * tmpSphere.m_Radius - m_Radius * m_Radius;
	}

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
	BVH_Node(BVH_Node<NodeData>* parent, NodeData* node, const BoundingSphere& volume);

	~BVH_Node();

	bool IsLeaf() const { return m_NodeData; }

private:
	void AddNode(NodeData* node, const BoundingSphere& volume);

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

	void AddNode(NodeData* node, const BoundingSphere& volume);

private:
	BVH_Node<NodeData>* m_Root;
};

#include "BVH_Tree.inl"


