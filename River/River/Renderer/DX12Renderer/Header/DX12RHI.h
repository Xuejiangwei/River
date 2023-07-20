#pragma once

#include "RHI.h"

#include <Windows.h>
#include <wrl.h>
#include <dxgi1_6.h>

#include <d3d12.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#include "Renderer/DX12Renderer/Header/UploadBuffer.h"

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

class DX12PipelineState;
class DX12UniformBuffer;
class DX12MeshGeometry;

struct ObjectConstants
{
	DirectX::XMFLOAT4X4 WorldViewProj = Identity4x4();
	DirectX::XMFLOAT4 Color;
};

class DX12RHI : public RHI
{
public:
	DX12RHI();
	virtual ~DX12RHI() override;

	virtual void Initialize(const RHIInitializeParam& param) override;

	virtual void Render() override;
	
	virtual void OnUpdate() override;

	virtual Share<class PipelineState> BuildPSO(Share<Shader> Shader, const Vector<ShaderLayout>& Layout) override;

	virtual Share<VertexBuffer> CreateVertexBuffer(float* vertices, uint32_t size, uint32_t elementSize, const VertexBufferLayout& layout) override;

	virtual Share<IndexBuffer> CreateIndexBuffer(uint32_t* indices, uint32_t count, ShaderDataType indiceDataType) override;

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

	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const
	{
		return m_DsvHeap->GetCPUDescriptorHandleForHeapStart();
	}

	void BuildDescriptorHeaps()
	{
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
		cbvHeapDesc.NumDescriptors = 1;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbvHeapDesc.NodeMask = 0;
		ThrowIfFailed(m_Device->CreateDescriptorHeap(&cbvHeapDesc,
			IID_PPV_ARGS(&m_CbvHeap)));
	}

	void BuildConstantBuffers();

	void BuildShadersAndInputLayout();

	void BuildRootSignature();

	void BuildPSO();

	void BuildTestVertexBufferAndIndexBuffer();

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

	DXGI_FORMAT	m_RenderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	Unique<class DX12FrameBuffer> m_FrameBuffers[s_SwapChainBufferCount];

	RHIInitializeParam m_InitParam;

	Unique<UploadBuffer<ObjectConstants>> m_ObjectCB = nullptr;

	Microsoft::WRL::ComPtr<ID3DBlob> m_vsByteCode = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> m_psByteCode = nullptr;
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputLayout;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PSO = nullptr;

};
