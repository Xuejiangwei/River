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

struct DX12RenderItem : public RenderItem111
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
	UINT SkinnedCBIndex = -1;
	struct SkinnedModelInstance* SkinnedModelInst = nullptr;

	class Material* Mat = nullptr;
	struct MeshGeometry* Geo = nullptr;

	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	bool Visible = true;
	DirectX::BoundingBox Bounds;
	V_Array<DX12InstanceUniformData> Instances;

};

Unique<DX12RenderItem> CreateDX12RenderItem(DirectX::XMMATRIX world, DirectX::XMMATRIX tex, uint32_t index, class Material* mat, 
	D3D12_PRIMITIVE_TOPOLOGY primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
void SetRenderItemGeo(DX12RenderItem* renderItem, struct MeshGeometry* geo, const char* geoDrawArg = nullptr);