#include "BVH_Tree.h"

inline BoundingSphere::BoundingSphere(const Float3& center, float radius)
	: m_Center(center), m_Radius(radius)
{
}

inline BoundingSphere::BoundingSphere(const BoundingSphere& bs1, const BoundingSphere& bs2)
{
	Float3 centreOffset = bs2.m_Center - bs1.m_Center;
	float distance = centreOffset.LenSquared();
	float radiusDiff = bs2.m_Radius - bs1.m_Radius;

	// 如果半径差大于球心距离，说明一个球完全覆盖了另一个球
	if (radiusDiff * radiusDiff >= distance)
	{
		// 直接用半径大的球作为包围球
		if (bs1.m_Radius > bs2.m_Radius)
		{
			m_Center = bs1.m_Center;
			m_Radius = bs1.m_Radius;
		}
		else
		{
			m_Center = bs2.m_Center;
			m_Radius = bs2.m_Radius;
		}
	}
	// 两个球部分相交或完全不相交
	else
	{
		distance = sqrt(distance);
		m_Radius = (distance + bs1.m_Radius + bs2.m_Radius) * 0.5f;

		// 新的球心位置是从原来的一个球心出发，向另一个球球心的方向移动一段距离
		// 距离为新球半径减去出发球半径
		if (distance > 0)
			m_Center = bs1.m_Center + (centreOffset / distance) * (m_Radius - bs1.m_Radius);
		else
			m_Center = bs1.m_Center;
	}
}

inline BoundingSphere::~BoundingSphere()
{
}

template<typename NodeData>
inline BVH_Node<NodeData>::BVH_Node(BVH_Node<NodeData>* parent, NodeData* node, const BoundingSphere& volume)
	: m_Parent(parent), m_NodeData(node), m_BoundingVolume(volume)
{
	m_Children[0] = m_Children[1] = nullptr;
}

template<typename NodeData>
inline BVH_Node<NodeData>::~BVH_Node()
{
	if (m_Parent != nullptr)
	{
		// 先找到兄弟节点
		auto sibling = (m_Parent->m_Children[0] == this) ? m_Parent->m_Children[1] : m_Parent->m_Children[0];

		// 把兄弟节点的数据复制到父节点
		m_Parent->m_NodeData = sibling->m_NodeData;
		m_Parent->m_Children[0] = sibling->m_Children[0];
		m_Parent->m_Children[1] = sibling->m_Children[1];
		m_Parent->m_BoundingVolume = sibling->m_BoundingVolume;

		// 如果父节点有刚体，同步更新刚体的BVHNode指针
		if (m_Parent->m_NodeData)
			m_Parent->m_NodeData->m_BVH_Node = m_Parent;

		// 然后把兄弟节点删除
		sibling->m_Parent = nullptr;
		sibling->m_NodeData = nullptr;
		sibling->m_Children[0] = nullptr;
		sibling->m_Children[1] = nullptr;
		delete sibling;

		// 最后重新计算父节点的BV
		m_Parent->UpdateBoundingVolume();
	}

	// 如果有刚体，并且引用了当前节点，就消除引用避免野指针
	if (m_NodeData && m_NodeData->m_BVH_Node == this)
		m_NodeData->m_BVH_Node = nullptr;

	// 递归删除子节点，删之前先把父节点引用清空，这样子节点删除时无需再处理兄弟节点
	if (m_Children[0] != nullptr)
	{
		m_Children[0]->m_Parent = nullptr;
		delete m_Children[0];
	}
	if (m_Children[1] != nullptr)
	{
		m_Children[1]->m_Parent = nullptr;
		delete m_Children[1];
	}
}

template<typename NodeData>
inline void BVH_Node<NodeData>::AddNode(NodeData* node, const BoundingSphere& volume)
{
	if (IsLeaf())
	{
		m_Children[0] = new BVH_Node<NodeData>(this, m_NodeData, m_BoundingVolume);
		m_Children[1] = new BVH_Node<NodeData>(this, node, volume);;

		m_NodeData = nullptr;
		UpdateBoundingVolume();
	}
	else
	{
		if (m_Children[0]->m_BoundingVolume.GetGrowth(volume) < m_Children[1]->m_BoundingVolume.GetGrowth(volume))
		{
			m_Children[0]->AddNode(node, volume);
		}
		else
		{
			m_Children[1]->AddNode(node, volume);
		}
	}
}

template<typename NodeData>
inline void BVH_Node<NodeData>::UpdateBoundingVolume(bool recurse)
{
	// 叶子节点只包含一个刚体，目前的物理系统默认刚体不会变，所以无需重新计算BV大小
	if (!IsLeaf())
		m_BoundingVolume = BoundingSphere(m_Children[0]->m_BoundingVolume, m_Children[1]->m_BoundingVolume);

	// 如果需要递归，就递归更新父节点的BV
	if (m_Parent != nullptr && recurse)
		m_Parent->UpdateBoundingVolume(true);
}

template<typename NodeData>
inline BVH_Tree<NodeData>::BVH_Tree(BVH_Node<NodeData>* node)
	: m_Root(node)
{
}

template<typename NodeData>
inline BVH_Tree<NodeData>::~BVH_Tree()
{
	if (m_Root)
	{
		delete m_Root;
	}
}

template<typename NodeData>
inline void BVH_Tree<NodeData>::AddNode(NodeData* node, const BoundingSphere& volume)
{
	m_Root->AddNode(node, volume);
}
