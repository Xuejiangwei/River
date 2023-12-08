#pragma once

//#include "RenderPass.h"
#include "DX12Util.h"
#include "d3dx12.h"

class DX12ShadowPass// : public RenderPass
{
public:
	DX12ShadowPass(ID3D12Device* device, UINT width, UINT height);

	virtual ~DX12ShadowPass();

	virtual void Initialize();

	virtual void Draw();

	virtual void Update(class FrameBuffer* frameBuffer);

	virtual void OnResize(int width, int height);

	ID3D12Resource* Resource() { return m_ShadowMap.Get(); }

	void BuildResource();

	void BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuSrv, CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuSrv, CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDsv);

private:
	void BuildSrvAndDsv();

private:
	D3D12_VIEWPORT m_Viewport;
	D3D12_RECT m_ScissorRect;

	ID3D12Device* m_Device;
	UINT m_Width, m_Height;
	DXGI_FORMAT m_Format = DXGI_FORMAT_R24G8_TYPELESS;

	CD3DX12_CPU_DESCRIPTOR_HANDLE m_HandleCpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_HandleGpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_HandleCpuDsv;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_ShadowMap;
};
