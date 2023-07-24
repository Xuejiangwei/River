#pragma once

#include "RHI.h"

#include <Windows.h>
#include <wrl.h>
#include <dxgi1_6.h>

#include <d3d12.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#include "Event.h"
#include "Material.h"
#include "Renderer/DX12Renderer/Header/DX12Camera.h"
#include "Renderer/DX12Renderer/Header/UploadBuffer.h"
#include "Renderer/DX12Renderer/Header/DX12UniformBuffer.h"
#include "Renderer/DX12Renderer/Header/DX12FrameBuffer.h"

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

class DX12Texture;
class DX12Shader;
class DX12PipelineState;

class DX12RHI : public RHI
{
public:
	DX12RHI();
	virtual ~DX12RHI() override;

	virtual void Initialize(const RHIInitializeParam& param) override;

	virtual void Render() override;

	virtual void OnUpdate() override;

	virtual Share<class PipelineState> BuildPSO(Share<Shader> Shader, const V_Array<ShaderLayout>& Layout) override;

	virtual Share<VertexBuffer> CreateVertexBuffer(float* vertices, uint32_t size, uint32_t elementSize, const VertexBufferLayout& layout) override;

	virtual Share<IndexBuffer> CreateIndexBuffer(uint32_t* indices, uint32_t count, ShaderDataType indiceDataType) override;

	virtual void Resize(const RHIInitializeParam& param) override;

	virtual Camera* GetMainCamera() override;

private:
	void LoadTextures();

	void InitBaseMaterials();

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

	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const
	{
		return m_DsvHeap->GetCPUDescriptorHandleForHeapStart();
	}

	void BuildDescriptorHeaps();

	void BuildTestVertexBufferAndIndexBuffer();

	void InitFrameBuffer();

	void BuildConstantBufferViews();

private:
	Microsoft::WRL::ComPtr<IDXGIFactory5> m_Factory;
	Microsoft::WRL::ComPtr<ID3D12Device4> m_Device;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;

	static const int s_SwapChainBufferCount = 2;
	static const int s_FrameBufferCount = 3;		//因为GBuffer的原因,可能会更大
	static const int s_MaxRenderItem = 10;
	static const int s_BaseMaterialCount = 1;
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

	DX12Camera m_PrespectiveCamera;
	DX12Camera m_OrthoGraphicCamera;

	V_Array<Unique<DX12FrameBuffer>> m_FrameBuffer;
	DX12FrameBuffer* m_CurrFrameResource = nullptr;
	int m_CurrFrameResourceIndex = 0;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_CbvHeap;
	PassUniform m_MainPassUniformData;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_SrvDescriptorHeap;

	Share<DX12Shader> m_Shader;
	V_Array<Share<DX12PipelineState>> m_PSOs;

	DXGI_FORMAT	m_RenderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	RHIInitializeParam m_InitParam;
	V_Array<Material> m_BaseMaterials;
	V_Array<Unique<DX12Texture>> m_Textures;
};
