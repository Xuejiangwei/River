#include "RiverPch.h"
#include "Renderer/DX12Renderer/Header/DX12Texture.h"
#include "Renderer/DX12Renderer/Header/DDSTextureLoader.h"
#include "Renderer/DX12Renderer/Header/DX12DescriptorAllocator.h"
#include "Utils/Header/StringUtils.h"

DX12Texture::DX12Texture(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const String& name, const String& path, Type type)
	: Texture(name, path, type)
{
	auto ws = S_2_WS(path);
	DirectX::CreateDDSTextureFromFile12(device, commandList, ws.c_str(), m_Resource, m_UploadHeap);
	m_DescriptorHandle = DX12DescriptorAllocator::Allocate(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).ptr;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = IsCubeTexture() ? D3D12_SRV_DIMENSION_TEXTURECUBE : D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Format = m_Resource->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = m_Resource->GetDesc().MipLevels;

	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	handle.ptr = m_DescriptorHandle;
	device->CreateShaderResourceView(m_Resource.Get(), &srvDesc, handle);
}

DX12Texture::DX12Texture(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const String& name, const uint8* data, int width, int height)
	: Texture(name, "", Type::Texture2D)
{
	D3D12_HEAP_PROPERTIES props;
	memset(&props, 0, sizeof(D3D12_HEAP_PROPERTIES));
	props.Type = D3D12_HEAP_TYPE_DEFAULT;
	props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Alignment = 0;
	desc.Width = width;
	desc.Height = height;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc,
		D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_Resource));

	UINT uploadPitch = (width * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u);
	UINT uploadSize = height * uploadPitch;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Alignment = 0;
	desc.Width = uploadSize;
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	props.Type = D3D12_HEAP_TYPE_UPLOAD;
	props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_UploadHeap));

	void* mapped = nullptr;
	D3D12_RANGE range = { 0, uploadSize };
	m_UploadHeap->Map(0, &range, &mapped);
	for (int y = 0; y < height; y++)
		memcpy((void*)((uintptr_t)mapped + y * uploadPitch), data + y * width * 4, width * 4);
	m_UploadHeap->Unmap(0, &range);

	D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
	srcLocation.pResource = m_UploadHeap.Get();
	srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	srcLocation.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srcLocation.PlacedFootprint.Footprint.Width = width;
	srcLocation.PlacedFootprint.Footprint.Height = height;
	srcLocation.PlacedFootprint.Footprint.Depth = 1;
	srcLocation.PlacedFootprint.Footprint.RowPitch = uploadPitch;

	D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
	dstLocation.pResource = m_Resource.Get();
	dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dstLocation.SubresourceIndex = 0;

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_Resource.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	commandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
	commandList->ResourceBarrier(1, &barrier);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Format = m_Resource->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = m_Resource->GetDesc().MipLevels;

	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	m_DescriptorHandle = DX12DescriptorAllocator::Allocate(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).ptr;
	handle.ptr = m_DescriptorHandle;
	device->CreateShaderResourceView(m_Resource.Get(), &srvDesc, handle);

	//m_UploadHeap->Release();
	//m_Resource->Release();
}

DX12Texture::DX12Texture(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const String& name, int width, int height)
	: Texture(name, "", Type::Texture2D)
{
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&optClear,
		IID_PPV_ARGS(&m_Resource));


	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.PlaneSlice = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	m_DescriptorHandle = DX12DescriptorAllocator::Allocate(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).ptr;
	handle.ptr = m_DescriptorHandle;
	device->CreateShaderResourceView(m_Resource.Get(), &srvDesc, handle);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.Texture2D.MipSlice = 0;

	m_DSV_DescriptorHandle = DX12DescriptorAllocator::Allocate(D3D12_DESCRIPTOR_HEAP_TYPE_DSV).ptr;
	handle.ptr = m_DSV_DescriptorHandle;
	device->CreateDepthStencilView(m_Resource.Get(), &dsvDesc, handle);
}

DX12Texture::~DX12Texture()
{
}