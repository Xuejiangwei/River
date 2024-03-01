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

	// ����뾶��������ľ��룬˵��һ������ȫ��������һ����
	if (radiusDiff * radiusDiff >= distance)
	{
		// ֱ���ð뾶�������Ϊ��Χ��
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
	// �����򲿷��ཻ����ȫ���ཻ
	else
	{
		distance = sqrt(distance);
		m_Radius = (distance + bs1.m_Radius + bs2.m_Radius) * 0.5f;

		// �µ�����λ���Ǵ�ԭ����һ�����ĳ���������һ�������ĵķ����ƶ�һ�ξ���
		// ����Ϊ����뾶��ȥ������뾶
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
		// ���ҵ��ֵܽڵ�
		auto sibling = (m_Parent->m_Children[0] == this) ? m_Parent->m_Children[1] : m_Parent->m_Children[0];

		// ���ֵܽڵ�����ݸ��Ƶ����ڵ�
		m_Parent->m_NodeData = sibling->m_NodeData;
		m_Parent->m_Children[0] = sibling->m_Children[0];
		m_Parent->m_Children[1] = sibling->m_Children[1];
		m_Parent->m_BoundingVolume = sibling->m_BoundingVolume;

		// ������ڵ��и��壬ͬ�����¸����BVHNodeָ��
		if (m_Parent->m_NodeData)
			m_Parent->m_NodeData->m_BVH_Node = m_Parent;

		// Ȼ����ֵܽڵ�ɾ��
		sibling->m_Parent = nullptr;
		sibling->m_NodeData = nullptr;
		sibling->m_Children[0] = nullptr;
		sibling->m_Children[1] = nullptr;
		delete sibling;

		// ������¼��㸸�ڵ��BV
		m_Parent->UpdateBoundingVolume();
	}

	// ����и��壬���������˵�ǰ�ڵ㣬���������ñ���Ұָ��
	if (m_NodeData && m_NodeData->m_BVH_Node == this)
		m_NodeData->m_BVH_Node = nullptr;

	// �ݹ�ɾ���ӽڵ㣬ɾ֮ǰ�ȰѸ��ڵ�������գ������ӽڵ�ɾ��ʱ�����ٴ����ֵܽڵ�
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
	// Ҷ�ӽڵ�ֻ����һ�����壬Ŀǰ������ϵͳĬ�ϸ��岻��䣬�����������¼���BV��С
	if (!IsLeaf())
		m_BoundingVolume = BoundingSphere(m_Children[0]->m_BoundingVolume, m_Children[1]->m_BoundingVolume);

	// �����Ҫ�ݹ飬�͵ݹ���¸��ڵ��BV
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
