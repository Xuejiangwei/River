#pragma once

#include "RiverHead.h"
#include "RenderItem.h"
#include "DX12Util.h"
#include "DirectXCollision.h"

struct DX12InstanceUniformData
{
	DirectX::XMFLOAT4X4 World = Identity4x4();
	DirectX::XMFLOAT4X4 TexTransform = Identity4x4();
	UINT MaterialIndex;
	UINT InstancePad0;
	UINT InstancePad1;
	UINT InstancePad2;
};

struct DX12RenderItem : public RenderItem
{
	DX12RenderItem() = default;

	DX12RenderItem(const DX12RenderItem& rhs) = delete;
	
	DirectX::XMFLOAT4X4 World = Identity4x4();
	DirectX::XMFLOAT4X4 TexTransform = Identity4x4();

	int NumFramesDirty = 3;
	UINT ObjCBIndex = -1;
	int InstanceCount = 1;
	int IndexCount = 0;
	int StartIndexLocation = 0;
	int BaseVertexLocation = 0;

	class Material* Mat = nullptr;
	struct MeshGeometry* Geo = nullptr;

	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	bool Visible = true;
	DirectX::BoundingBox Bounds;
	V_Array<DX12InstanceUniformData> Instances;

};