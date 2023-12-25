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

class DX12DynamicDescriptorHeap;
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
	friend static Texture* Texture::CreateImmediatelyTexture(const char* name, const char* filePath);
public:
	DX12RHI();
	
	virtual ~DX12RHI() override;

	virtual void Initialize(const RHIInitializeParam& param) override;

	virtual void Exit() override;

	virtual void BeginFrame() override;

	virtual void EndFrame() override;

	virtual void Render() override;

	virtual void OnUpdate(const RiverTime& time) override;

	virtual void UpdateSceneData(const V_Array<RawVertex>& vertices, const V_Array<uint16_t> indices) override;

	virtual void UpdateUIData(V_Array<UIVertex>& vertices, V_Array<uint16> indices) override;

	virtual void SetUpStaticMesh(StaticMesh* mesh) override;

	virtual void SetUpMaterial(Material* material) override;

	virtual Pair<void*, void*> GetStaticMeshBuffer(const char* name) override;

	virtual Material* CreateMaterial(const char* name) override;

	virtual void SetViewPort(uint32 w, uint32 h, uint32 xOffset = 0, uint32 yOffset = 0) override;

	virtual Texture* GetTexture(const char* name) override;

	virtual void GenerateDrawCommands(int commandId) override;

	virtual int AllocDrawCommand() override;

	virtual void DrawRenderItem(int renderItemId) override;

	DX12Texture* CreateTexture(const char* name, const char* filePath);

	ID3D12Device* GetDevice() { return m_Device.Get(); }

	//测试
	void AddDescriptor(DX12Texture* texture);

	DX12Texture* CreateTexture(const char* name, int width, int height, const uint8* data);

	void RemoveTexture(const String& name);
	
	Unique<DX12PipelineState> CreatePSO(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc, const V_Array<D3D12_INPUT_ELEMENT_DESC>* layout, Shader* vsShader, Shader* psShader);

	Unique<DX12VertexBuffer> CreateVertexBuffer(void* vertices, uint32_t byteSize, uint32_t elementSize, const V_Array<D3D12_INPUT_ELEMENT_DESC>* layout);

	Unique<DX12VertexBuffer> CreateUploadVertexBuffer(void* vertices, uint32_t byteSize, uint32_t elementSize, const V_Array<D3D12_INPUT_ELEMENT_DESC>* layout);

	Unique<DX12IndexBuffer> CreateIndexBuffer(void* indices, uint32_t count, ShaderDataType indiceDataType);

	Unique<DX12IndexBuffer> CreateUploadIndexBuffer(void* indices, uint32_t count, ShaderDataType indiceDataType);

	virtual void Resize(const RHIInitializeParam& param) override;

	virtual Camera* GetMainCamera() override;

	virtual void Pick(int x, int y) override;

	void WaitFence();

	void ResetCmdListAlloc();

	void ExecuteCmdList(bool isSwapChain = true);

private:
	void InitializeBase(const RHIInitializeParam& param);

	//void LoadSkinnedModel();

	void InitBaseGeometry();

	void InitBaseTexture();

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

	void UpdateShadowTransform(const RiverTime& time);

	void UpdateObjectCBs();

	void UpdateSkinnedCBs(const RiverTime& time);

	void UpdateMaterialCBs();

	void UpdateMainPass(/*const RiverTime& time*/);

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

	void InitFrameBuffer();

	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const V_Array<DX12RenderItem*>& items);

	void DrawSceneToShadowMap();

	void DrawNormalsAndDepth();

	void DrawUI();

	void CreateSRV(CD3DX12_CPU_DESCRIPTOR_HANDLE& handle, ID3D12Resource* textureRes, D3D12_SHADER_RESOURCE_VIEW_DESC& desc, uint32_t handleOffset = 0);

public:
	static const int s_SwapChainBufferCount = 2;

private:
	Microsoft::WRL::ComPtr<IDXGIFactory5> m_Factory;
	Microsoft::WRL::ComPtr<ID3D12Device> m_Device;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;

	/** 在每帧渲染时将每个RenderItem的Texture资源（每个Texture中含有描述符handle，handle中有texture在内存中的指针）的handle复制到其中
		使其可以在Shader代码中根据描述符的texture槽取texture资源时中可以连续并从头（即第0个）访问。
	**/
	Unique<DX12DynamicDescriptorHeap> m_DynamicDescriptorHeaps[RHI::GetFrameCount()];

	int m_CurrFrameResourceIndex;
	int m_CurrBackBufferIndex;

	Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;

	ID3D12DescriptorHeap* m_RtvDescriptorHeap;
	ID3D12DescriptorHeap* m_DsvHeap;
	ID3D12DescriptorHeap* m_SrvDescriptorHeap;
	//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_CbvHeap;

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
	V_Array<Unique<DX12FrameBuffer>> m_FrameBuffer;
	V_Array<DX12RenderItem*> m_RitemLayer[(int)RenderLayer::LayerCount];
	HashMap<String, Unique<DX12PipelineState>> m_PSOs;
	HashMap<String, V_Array<D3D12_INPUT_ELEMENT_DESC>> m_InputLayers;
	HashMap<String, Pair<Unique<DX12VertexBuffer>, Unique<DX12IndexBuffer>>> m_MeshBuffer;

	RenderItem m_UIRenderItem;

	Unique<ShadowMap> m_ShadowMap;
	std::unique_ptr<Ssao> m_Ssao;
	PassUniform m_MainPassCB;
	PassUniform m_ShadowPassCB;

	V_Array<int> m_DrawItems;

	Unique<DX12VertexBuffer> m_RawMeshVertexBuffer;
	Unique<DX12IndexBuffer> m_RawMeshIndexBuffer;
	Unique<DX12VertexBuffer> m_UIVertexBuffer;
	Unique<DX12IndexBuffer> m_UIIndexBuffer;
};
