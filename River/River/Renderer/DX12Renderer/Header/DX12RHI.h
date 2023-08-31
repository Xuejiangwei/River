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
#include "Renderer/DX12Renderer/Header/DX12RenderItem.h"
#include "Renderer/DX12Renderer/Header/DX12Camera.h"
#include "Renderer/DX12Renderer/Header/UploadBuffer.h"
#include "Renderer/DX12Renderer/Header/DX12UniformBuffer.h"
#include "Renderer/DX12Renderer/Header/DX12FrameBuffer.h"
#include "Renderer/DX12Renderer/Header/DX12Util.h"

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

class ShadowMap;
class DX12VertexBuffer;
class DX12IndexBuffer;
class DX12Texture;
class DX12Shader;
class DX12RootSignature;
class DX12PipelineState;

using namespace DirectX;


class DX12RHI : public RHI
{
public:
	DX12RHI();
	virtual ~DX12RHI() override;

	virtual void Initialize(const RHIInitializeParam& param) override;

	virtual void Render() override;

	virtual void OnUpdate(const RiverTime& time) override;

	virtual void UpdateUIData(V_Array<UIVertex>& vertices, V_Array<uint16_t> indices) override;

	DX12Texture* CreateTexture(const char* name, const char* filePath);

	DX12Texture* CreateTexture(const char* name, int width, int height, const uint8* data);
	
	Unique<DX12PipelineState> CreatePSO(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc, const V_Array<D3D12_INPUT_ELEMENT_DESC>* layout, Shader* vsShader, Shader* psShader);

	Unique<DX12VertexBuffer> CreateVertexBuffer(void* vertices, uint32_t byteSize, uint32_t elementSize, const V_Array<D3D12_INPUT_ELEMENT_DESC>* layout);

	Unique<DX12VertexBuffer> CreateUploadVertexBuffer(void* vertices, uint32_t byteSize, uint32_t elementSize, const V_Array<D3D12_INPUT_ELEMENT_DESC>* layout);

	Unique<DX12IndexBuffer> CreateIndexBuffer(void* indices, uint32_t count, ShaderDataType indiceDataType);

	Unique<DX12IndexBuffer> CreateUploadIndexBuffer(void* indices, uint32_t count, ShaderDataType indiceDataType);

	virtual void Resize(const RHIInitializeParam& param) override;

	virtual Camera* GetMainCamera() override;

	virtual void Pick(int x, int y) override;

private:
	void InitializeBase(const RHIInitializeParam& param);

	void LoadSkinnedModel();

	void BuildShapeGeometry();

	void LoadTextures();

	void InitBaseMaterials();

	void InitBaseShaders();

	void InitBaseRootSignatures();

	void InitBasePSOs();

	void EnumAdaptersAndCreateDevice();

	void CreateCommandQueue();

	void CreateSwapChain();

	void CreateRtvAndDsvHeaps();

	void CreateFence();

	void CheckQualityLevel();

	void AnimationMaterials(const RiverTime& time);

	void UpdateShadowTransform(const RiverTime& time);

	void UpdateObjectCBs();

	void UpdateSkinnedCBs(const RiverTime& time);

	void UpdateMaterialCBs();

	void UpdateMainPass(const RiverTime& time);

	void UpdateShadowPass(const RiverTime& time);
	
	void UpdateSsaoCBs(const RiverTime& time);

	void FlushCommandQueue();

	ID3D12Resource* CurrentBackBuffer() const { return m_SwapChainBuffer[m_CurrBackBufferIndex].Get(); }

	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;

	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const
	{
		return m_DsvHeap->GetCPUDescriptorHandleForHeapStart();
	}

	void InitDescriptorHeaps();

	void InitBaseRenderItems();

	void InitFrameBuffer();

	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const V_Array<DX12RenderItem*>& items);

	void DrawSceneToShadowMap();

	void DrawNormalsAndDepth();

	void DrawUI();

	void CreateSRV(CD3DX12_CPU_DESCRIPTOR_HANDLE& handle, ID3D12Resource* textureRes, D3D12_SHADER_RESOURCE_VIEW_DESC& desc, uint32_t handleOffset = 0);

	CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuSrv(int index) const
	{
		auto srv = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_SrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		srv.Offset(index, m_CbvSrvUavDescriptorSize);
		return srv;
	}

	CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuSrv(int index) const
	{
		auto srv = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_SrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		srv.Offset(index, m_CbvSrvUavDescriptorSize);
		return srv;
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE GetRtv(int index)const
	{
		auto rtv = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		rtv.Offset(index, m_RtvDescriptorSize);
		return rtv;
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE GetDsv(int index) const
	{
		auto dsv = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_DsvHeap->GetCPUDescriptorHandleForHeapStart());
		dsv.Offset(index, m_DsvDescriptorSize);
		return dsv;
	}

public:
	static const int s_SwapChainBufferCount = 2;
	static const int s_FrameBufferCount = 3;
	static const int s_MaxRenderItem = 100;

private:
	Microsoft::WRL::ComPtr<IDXGIFactory5> m_Factory;
	Microsoft::WRL::ComPtr<ID3D12Device> m_Device;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
	
	int m_CurrFrameResourceIndex;
	int m_CurrBackBufferIndex;

	uint32 m_RtvDescriptorSize;
	uint32 m_DsvDescriptorSize;
	uint32 m_CbvSrvUavDescriptorSize;

	Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RtvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_SrvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_CbvHeap;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_SwapChainBuffer[s_SwapChainBufferCount];
	Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthStencilBuffer;

	Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
	UINT64 m_CurrentFence;
	HANDLE m_HandleEventFence = nullptr;

	D3D12_VIEWPORT m_ScreenViewport;
	D3D12_RECT m_ScissorRect;

	DX12Camera m_PrespectiveCamera;
	DX12Camera m_OrthoGraphicCamera;

	DX12FrameBuffer* m_CurrFrameResource = nullptr;

	DXGI_FORMAT	m_RenderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	RHIInitializeParam m_InitParam;
	HashMap<String, Unique<DX12Shader>> m_Shaders;
	HashMap<String, Unique<DX12RootSignature>> m_RootSignatures;
	HashMap<String, Unique<DX12Texture>> m_Textures;
	HashMap<String, std::unique_ptr<Material>> m_Materials;
	V_Array<Unique<DX12RenderItem>> m_AllRitems;
	V_Array<Unique<DX12FrameBuffer>> m_FrameBuffer;
	V_Array<DX12RenderItem*> m_RitemLayer[(int)RenderLayer::LayerCount];
	HashMap<String, Unique<DX12PipelineState>> m_PSOs;
	HashMap<String, V_Array<D3D12_INPUT_ELEMENT_DESC>> m_InputLayers;
	HashMap<String, Unique<MeshGeometry>> m_Geometries;

	DX12RenderItem* m_UIRenderItem;

	Unique<ShadowMap> m_ShadowMap;
	PassUniform m_MainPassCB;
	PassUniform m_ShadowPassCB;
};
