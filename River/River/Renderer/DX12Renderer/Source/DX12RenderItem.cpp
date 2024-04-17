#include "RiverPch.h"
#include "Renderer/DX12Renderer/Header/DX12RenderItem.h"
#include "Renderer/DX12Renderer/Header/DX12GeometryGenerator.h"

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
