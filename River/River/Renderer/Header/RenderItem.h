#pragma once
#include "RiverHead.h"


enum class PrimitiveType
{
	TriangleList
};

struct RenderItem
{
	Matrix4x4 World;
	Matrix4x4 TexTransform;


	void* VertexBuffer;
	void* IndexBuffer;
	class Material* Material;
	int NumFramesDirty;
	int InstanceCount;
	int IndexCount;
	int StartIndexLocation;
	int BaseVertexLocation;
	
	PrimitiveType PriType;

	uint32 ObjCBIndex = -1;
	uint32 SkinnedCBIndex = -1;

	RenderItem()
		: NumFramesDirty(3), InstanceCount(1), IndexCount(0), StartIndexLocation(0), BaseVertexLocation(0), 
		PriType(PrimitiveType::TriangleList), ObjCBIndex(-1), SkinnedCBIndex(-1),
		Material(nullptr), VertexBuffer(nullptr), IndexBuffer(nullptr),
		World(Matrix4x4::UnitMatrix()), TexTransform(Matrix4x4::UnitMatrix())
	{}
};

struct UIRenderItem
{
	enum class RenderFlag : uint32
	{
		None,
		PointColor = 1 << 0,
	};

	Matrix4x4 World;

	int IndexCount;
	int StartIndexLocation;
	int BaseVertexLocation;
	int ObjCBIndex;
	class Material* Material;
	class Texture* RenderTexture;
	uint32 RenderFlag;	//2的倍数  0 : 无   1 : 使用点的颜色

	UIRenderItem()
		: IndexCount(0), StartIndexLocation(0), BaseVertexLocation(0), ObjCBIndex(-1),
		World(Matrix4x4::UnitMatrix()), Material(nullptr), RenderTexture(nullptr), RenderFlag(0)
	{
	}
};
