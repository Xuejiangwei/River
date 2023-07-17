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

	virtual Share<class PipelineState> BuildPSO(Share<Shader> Shader, const Vector<ShaderLayout>& Layout) override;

	virtual Share<VertexBuffer> CreateVertexBuffer(float* vertices, uint32_t size, const VertexBufferLayout& layout) override;

	virtual void Resize(const RHIInitializeParam& param) override;

private:
	void CreateSwapChain(const RHIInitializeParam& param);

	void FlushCommandQueue();

	ID3D12Resource* CurrentBackBuffer() const { return m_SwapChainBuffer[m_CurrBackBuffer].Get(); }

	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;

	void BuildMeshGeometry();

private:
	Microsoft::WRL::ComPtr<IDXGIFactory4> m_Factory;
	Microsoft::WRL::ComPtr<ID3D12Device> m_Device;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
	
	static const int s_SwapChainBufferCount = 2;
	int m_CurrBackBuffer;
	UINT m_4xMsaaQuality;

	UINT m_RtvDescriptorSize;
	UINT m_DsvDescriptorSize;
	UINT m_CbvSrvUavDescriptorSize;

	Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;

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
};
