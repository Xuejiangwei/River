#include "RiverPch.h"
#include "Renderer/DX12Renderer/Header/DX12RenderItem.h"
#include "Renderer/DX12Renderer/Header/DX12GeometryGenerator.h"

Unique<DX12RenderItem> CreateDX12RenderItem(DirectX::XMMATRIX world, DirectX::XMMATRIX tex, uint32_t index, class Material* mat, D3D12_PRIMITIVE_TOPOLOGY primitiveType)
{
	auto renderItem = MakeUnique<DX12RenderItem>();
	XMStoreFloat4x4(&renderItem->World, world);
	XMStoreFloat4x4(&renderItem->TexTransform, tex);
	renderItem->ObjCBIndex = index;
	renderItem->Mat = mat;
	renderItem->PrimitiveType = primitiveType;

	return renderItem;
}

void SetRenderItemGeo(DX12RenderItem* renderItem, MeshGeometry* geo, const char* geoDrawArg)
{
	renderItem->Geo = geo;

	if (geoDrawArg)
	{
		renderItem->IndexCount = renderItem->Geo->DrawArgs[geoDrawArg].IndexCount;
		renderItem->StartIndexLocation = renderItem->Geo->DrawArgs[geoDrawArg].StartIndexLocation;
		renderItem->BaseVertexLocation = renderItem->Geo->DrawArgs[geoDrawArg].BaseVertexLocation;
		renderItem->Bounds = renderItem->Geo->DrawArgs[geoDrawArg].Bounds;
	}
	else
	{
		renderItem->IndexCount = 0;
		renderItem->StartIndexLocation = 0;
		renderItem->BaseVertexLocation = 0;
	}
}
