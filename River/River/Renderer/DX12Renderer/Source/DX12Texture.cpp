#include "RiverPch.h"
#include "Renderer/DX12Renderer/Header/DX12Texture.h"
#include "Renderer/DX12Renderer/Header/DDSTextureLoader.h"
#include "Utils/Header/StringUtils.h"

DX12Texture::DX12Texture(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const String& name, const String& path)
	: Texture(name, path)
{
	auto ws = S_2_WS(path);
	DirectX::CreateDDSTextureFromFile12(device, commandList, ws.c_str(), m_Resource, m_UploadHeap);
}

DX12Texture::~DX12Texture()
{
}