#pragma once

#include "Texture.h"

#include <wrl.h>
#include <d3d12.h>

class DX12Texture : public Texture
{
public:
	DX12Texture(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const String& name, const String& path);

	DX12Texture(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const String& name, const uint8* data, int width, int height);
	
	virtual ~DX12Texture() override;

	Microsoft::WRL::ComPtr<ID3D12Resource> GetResource() { return m_Resource; }

	Microsoft::WRL::ComPtr<ID3D12Resource> m_Resource;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_UploadHeap;
};
