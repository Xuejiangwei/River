#pragma once

#include "RHI.h"

#include <Windows.h>
#include <wrl.h>
#include <dxgi1_6.h>

#include <d3d12.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

class DX12PipelineState;
class DX12UniformBuffer;
class DX12MeshGeometry;

class DX12RHI : public RHI
{
public:
	DX12RHI();
	virtual ~DX12RHI() override;

	virtual void Initialize(const RHIInitializeParam& param) override;

	virtual void Render() override;
	
	virtual void OnUpdate() override;

	virtual Share<class PipelineState> BuildPSO(Share<Shader> Shader, const Vector<ShaderLayout>& Layout) override;

	virtual Share<VertexBuffer> CreateVertexBuffer(float* vertices, uint32_t size, const VertexBufferLayout& layout) override;

	virtual void Resize(const RHIInitializeParam& param) override;

private:
	void EnumAdaptersAndCreateDevice();

	void CreateCommandQueue();

	void CreateSwapChain();

	void CreateRtvAndDsvHeaps();

	void CreateFence();

	void CheckQualityLevel();

	void UpdateMainPass();

	void FlushCommandQueue();

	ID3D12Resource* CurrentBackBuffer() const { return m_SwapChainBuffer[m_CurrBackBuffer].Get(); }

	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;

	void BuildMeshGeometry(const String& MeshName);

private:
	Microsoft::WRL::ComPtr<IDXGIFactory5> m_Factory;
	Microsoft::WRL::ComPtr<ID3D12Device4> m_Device;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
	
	static const int s_SwapChainBufferCount = 2;
	int mCurrFrameResourceIndex = 0;
	int m_CurrBackBuffer;

	bool m_4xMsaaState = false;
	UINT m_4xMsaaQuality;

	UINT m_RtvDescriptorSize;
	UINT m_DsvDescriptorSize;
	UINT m_CbvSrvUavDescriptorSize;

	Microsoft::WRL::ComPtr<IDXGISwapChain1> m_SwapChain1;
	Microsoft::WRL::ComPtr<IDXGISwapChain3>	m_SwapChain3;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_SwapChainBuffer[s_SwapChainBufferCount];
	Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthStencilBuffer;

	Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
	UINT64 m_CurrentFence;
	HANDLE hEventFence = nullptr;

	D3D12_VIEWPORT m_Viewport;
	D3D12_RECT m_ScissorRect;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_CbvHeap;
	Unique<DX12UniformBuffer> m_UniformBuffer;

	Unique<DX12MeshGeometry> m_BoxGeo;

	std::vector<Share<DX12PipelineState>> m_PSOs;

	DXGI_FORMAT	m_RenderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	Unique<class DX12FrameBuffer> m_FrameBuffers[s_SwapChainBufferCount];

	RHIInitializeParam m_InitParam;
};
