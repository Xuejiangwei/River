#pragma once

#include "RiverHead.h"
#include "MathStruct.h"

class Material;
class VertexBuffer;
class IndexBuffer;

enum class PrimitiveType
{
	TriangleList
};

struct RenderItemInstance
{
	unsigned int IndexCount = 0;
	unsigned int StartIndexLocation = 0;
	int BaseVertexLocation = 0;

	River::Matrix4x4 m_WorldTransform;
	River::Matrix4x4 m_TexTransform;
	Material* m_Material;

	PrimitiveType m_PrimitiveType;

	unsigned int m_ObjectIndex;
	int NumFramesDirty = 3;

	// Bounding box of the geometry defined by this submesh. 
	// This is used in later chapters of the book.
	//DirectX::BoundingBox Bounds;
};

class RenderItem
{
public:
	RenderItem();

	~RenderItem();

public:
	VertexBuffer* m_VertexBuffer;
	IndexBuffer* m_IndexBuffer;
	unsigned int m_IndexCount;

	HashMap<String, RenderItemInstance> m_RenderInstance;
};
