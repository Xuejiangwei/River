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

	virtual Unique<class PipelineState> BuildPSO(Share<Shader> vsShader, Share<Shader> psShader, const V_Array<ShaderLayout>& Layout) override;

	virtual Unique<VertexBuffer> CreateVertexBuffer(float* vertices, uint32_t byteSize, uint32_t elementSize, const VertexBufferLayout& layout) override;

	virtual Unique<IndexBuffer> CreateIndexBuffer(void* indices, uint32_t count, ShaderDataType indiceDataType) override;

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

	ID3D12Resource* CurrentBackBuffer() const { return m_SwapChainBuffer[mCurrBackBuffer].Get(); }

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

	void CreateSRV(CD3DX12_CPU_DESCRIPTOR_HANDLE& handle, ID3D12Resource* textureRes, D3D12_SHADER_RESOURCE_VIEW_DESC& desc, uint32_t handleOffset = 0);

	CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuSrv(int index) const
	{
		auto srv = CD3DX12_CPU_DESCRIPTOR_HANDLE(mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		srv.Offset(index, mCbvSrvUavDescriptorSize);
		return srv;
	}

	CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuSrv(int index) const
	{
		auto srv = CD3DX12_GPU_DESCRIPTOR_HANDLE(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		srv.Offset(index, mCbvSrvUavDescriptorSize);
		return srv;
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE GetRtv(int index)const
	{
		auto rtv = CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		rtv.Offset(index, mRtvDescriptorSize);
		return rtv;
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE GetDsv(int index) const
	{
		auto dsv = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_DsvHeap->GetCPUDescriptorHandleForHeapStart());
		dsv.Offset(index, m_DsvDescriptorSize);
		return dsv;
	}

private:
	Microsoft::WRL::ComPtr<IDXGIFactory5> m_Factory;
	Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;

	static const int SwapChainBufferCount = 2;
	static const int s_FrameBufferCount = 3;		//因为GBuffer的原因,可能会更大
	static const int s_MaxRenderItem = 100;
	int mCurrFrameResourceIndex = 0;
	int mCurrBackBuffer;

	UINT mRtvDescriptorSize;
	UINT m_DsvDescriptorSize;
	UINT mCbvSrvUavDescriptorSize;

	Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_SwapChainBuffer[SwapChainBufferCount];
	Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;

	Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
	UINT64 mCurrentFence;
	HANDLE hEventFence = nullptr;

	D3D12_VIEWPORT mScreenViewport;
	D3D12_RECT mScissorRect;

	DX12Camera m_PrespectiveCamera;
	DX12Camera m_OrthoGraphicCamera;

	DX12FrameBuffer* mCurrFrameResource = nullptr;
	//int mCurrFrameResourceIndex = 0;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_CbvHeap;

	DXGI_FORMAT	mRenderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	RHIInitializeParam m_InitParam;
	HashMap<String, Unique<DX12Shader>> mShaders;
	HashMap<String, Unique<DX12RootSignature>> m_RootSignatures;
	HashMap<String, Microsoft::WRL::ComPtr<ID3D12PipelineState>> mPSOs;
	HashMap<String, Unique<DX12Texture>> mTextures;
	HashMap<String, std::unique_ptr<Material>> mMaterials;
	V_Array<Unique<DX12RenderItem>> mAllRitems;
	V_Array<Unique<DX12FrameBuffer>> mFrameBuffer;
	V_Array<DX12RenderItem*> mRitemLayer[(int)RenderLayer::LayerCount];
};
