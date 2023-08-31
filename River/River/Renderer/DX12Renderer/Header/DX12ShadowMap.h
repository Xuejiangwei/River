#pragma once

#include "DX12Util.h"
#include "d3dx12.h"

class ShadowMap
{
public:
	ShadowMap(ID3D12Device* device, uint32 width, uint32 height);

	~ShadowMap();
	
	ShadowMap(const ShadowMap& rhs) = delete;
	
	ShadowMap& operator=(const ShadowMap& rhs) = delete;

	uint32 Width() const { return m_Width; }
	
	uint32 Height() const { return m_Height; }
	
	ID3D12Resource* Resource() { return m_ShadowMap.Get(); }
	
	CD3DX12_GPU_DESCRIPTOR_HANDLE Srv() const { return m_HandleGpuSrv; }
	
	CD3DX12_CPU_DESCRIPTOR_HANDLE Dsv() const { return m_HandleCpuDsv; }

	D3D12_VIEWPORT Viewport() const { return m_Viewport; }

	D3D12_RECT ScissorRect() const { return m_ScissorRect; }

	void BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuSrv, CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuSrv, CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDsv);

	void OnResize(UINT newWidth, UINT newHeight);

private:
	void BuildDescriptors();

	void BuildResource();

private:
	ID3D12Device* m_Device;

	D3D12_VIEWPORT m_Viewport;
	D3D12_RECT m_ScissorRect;

	uint32 m_Width = 0;
	uint32 m_Height = 0;
	DXGI_FORMAT m_Format;

	CD3DX12_CPU_DESCRIPTOR_HANDLE m_HandleCpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_HandleGpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_HandleCpuDsv;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_ShadowMap;
};