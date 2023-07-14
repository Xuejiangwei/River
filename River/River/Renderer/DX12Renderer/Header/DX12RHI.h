#pragma once

#include "RHI.h"

#include <Windows.h>
#include <wrl.h>
#include <dxgi1_6.h>

#include <d3d12.h>

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")


class DX12RHI : public RHI
{
public:
	DX12RHI();
	virtual ~DX12RHI() override;

	virtual void Initialize(const RHIInitializeParam& Param) override;

	virtual Share<class PipelineState> BuildPSO(Share<Shader> Shader, const Vector<ShaderLayout>& Layout) override;

	virtual Share<VertexBuffer> CreateVertexBuffer(float* vertices, uint32_t size, const VertexBufferLayout& layout) override;

	virtual void Resize() override;

private:
	Microsoft::WRL::ComPtr<IDXGIFactory4> m_Factory;
	Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
	
	static const int s_SwapChainCount = 2;
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_SwapChainBuffer[s_SwapChainCount];

	Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
	UINT64 n64FenceValue = 0;
	HANDLE hEventFence = nullptr;

	D3D12_VIEWPORT m_ViewPort;
	D3D12_RECT m_ScissorRect;
};
