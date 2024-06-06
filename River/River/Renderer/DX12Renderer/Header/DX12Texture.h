#pragma once

#include "Renderer/Texture/Header/Texture.h"

#include <wrl.h>
#include <d3d12.h>

class DX12Texture : public Texture
{
public:
	DX12Texture(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const String& name, const String& path, Type type = Type::Texture2D);

	DX12Texture(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const String& name, const uint8* data, int width, int height, Type type = Type::Texture2D);

	DX12Texture(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const String& name, int width, int height, Type type = Type::Texture2D);

	virtual ~DX12Texture() override;

	virtual int GetTextureWidth() override { return (int)m_Resource->GetDesc().Width; }

	virtual int GetTextureHeight() override { return (int)m_Resource->GetDesc().Height; }

	Microsoft::WRL::ComPtr<ID3D12Resource> GetResource() { return m_Resource; }

	Microsoft::WRL::ComPtr<ID3D12Resource> m_Resource;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_UploadHeap;

	uint64 m_DSV_DescriptorHandle;
};
