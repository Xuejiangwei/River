#pragma once
#include "RiverHead.h"
#include "MathStruct.h"

enum class PrimitiveType
{
	TriangleList
};

struct RenderItem
{
	Matrix_4_4 World;
	Matrix_4_4 TexTransform;


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
		World(Matrix_4_4::UnitMatrix()), TexTransform(Matrix_4_4::UnitMatrix())
	{}
};

struct UIRenderItem
{
	Matrix_4_4 World;

	int IndexCount;
	int StartIndexLocation;
	int BaseVertexLocation;
	int ObjCBIndex;
	class Material* Material;
	class Texture* RenderTexture;

	UIRenderItem()
		: IndexCount(0), StartIndexLocation(0), BaseVertexLocation(0), ObjCBIndex(-1), Material(nullptr),
		World(Matrix_4_4::UnitMatrix()), RenderTexture(nullptr)
	{}
};
