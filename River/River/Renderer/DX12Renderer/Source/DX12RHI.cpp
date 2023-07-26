#include "RiverPch.h"
#include "RendererUtil.h"
#include "Renderer/DX12Renderer/Header/DX12Util.h"
#include "Renderer/DX12Renderer/Header/d3dx12.h"
#include "Renderer/DX12Renderer/Header/DX12RHI.h"
#include "Renderer/DX12Renderer/Header/DX12RootSignature.h"
#include "Renderer/DX12Renderer/Header/DX12PipelineState.h"
#include "Renderer/DX12Renderer/Header/DX12VertexBuffer.h"
#include "Renderer/DX12Renderer/Header/DX12IndexBuffer.h"
#include "Renderer/DX12Renderer/Header/DX12Shader.h"
#include "Renderer/DX12Renderer/Header/DX12GeometryGenerator.h"
#include "Renderer/DX12Renderer/Header/DX12Texture.h"
#include "Renderer/DX12Renderer/Header/Waves.h"

#include "DirectXMath.h"
#include <d3dcompiler.h>
#include <iostream>
#include <chrono>

#define DEFAULT_SHADER_PATH "F:\\GitHub\\River\\River\\Shaders\\Default.hlsl"
#define DEFAULT_TEXTURE_PATH_1 "F:\\GitHub\\River\\River\\Textures\\WoodCrate01.dds"
#define DEFAULT_TEXTURE_PATH_2 "F:\\GitHub\\River\\River\\Textures\\stone.dds"
#define DEFAULT_TEXTURE_PATH_3 "F:\\GitHub\\River\\River\\Textures\\tile.dds"
#define DEFAULT_TEXTURE_PATH_4 "F:\\GitHub\\River\\River\\Textures\\grass.dds"
#define DEFAULT_TEXTURE_PATH_5 "F:\\GitHub\\River\\River\\Textures\\water1.dds"
#define DEFAULT_TEXTURE_PATH_6 "F:\\GitHub\\River\\River\\Textures\\WireFence.dds"
#define DEFAULT_TEXTURE_PATH_7 "F:\\GitHub\\River\\River\\Textures\\checkboard.dds"
#define DEFAULT_TEXTURE_PATH_8 "F:\\GitHub\\River\\River\\Textures\\ice.dds"
#define DEFAULT_TEXTURE_PATH_9 "F:\\GitHub\\River\\River\\Textures\\white1x1.dds"

DX12RHI::DX12RHI()
	: m_CurrentFence(1), m_CurrBackBuffer(0), m_RtvDescriptorSize(0), m_DsvDescriptorSize(0), m_CbvSrvUavDescriptorSize(0),
	m_PrespectiveCamera(CameraType::Perspective), m_OrthoGraphicCamera(CameraType::OrthoGraphic)
{
}

DX12RHI::~DX12RHI()
{
	if (m_Device)
	{
		FlushCommandQueue();
	}
}

void DX12RHI::Initialize(const RHIInitializeParam& param)
{
	m_InitParam = param;

	m_PrespectiveCamera.SetPosition(0.0f, 0.0f, 0.0f);

	InitializeBase(param);

	ThrowIfFailed(m_CommandList->Reset(m_CommandAllocator.Get(), nullptr));

	{
		LoadTextures();

		InitBaseRootSignatures();
		
		InitDescriptorHeaps();
		
		InitBaseShaders();
		
		InitBaseVertexBufferAndIndexBuffer();
		
		InitBaseMaterials();
		
		InitRenderLayers();
		
		InitFrameBuffer();

		InitBasePSOs();
	}

	ThrowIfFailed(m_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	FlushCommandQueue();
}

void DX12RHI::OnUpdate(const RiverTime& time)
{
	m_PrespectiveCamera.OnUpdate();

	m_CurrFrameResourceIndex = (m_CurrFrameResourceIndex + 1) % s_FrameBufferCount;
	m_CurrFrameResource = m_FrameBuffer[m_CurrFrameResourceIndex].get();
	if (m_CurrFrameResource->m_FenceValue != 0 && m_Fence->GetCompletedValue() < m_CurrFrameResource->m_FenceValue)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

		ThrowIfFailed(m_Fence->SetEventOnCompletion(m_CurrFrameResource->m_FenceValue, eventHandle));

		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	AnimationMaterials(time);
	UpdateObjectCBs();
	UpdateMaterialCBs();

	UpdateMainPass(time);
}

void DX12RHI::Render()
{
	auto cmdListAlloc = m_CurrFrameResource->m_CommandAlloc;

	ThrowIfFailed(cmdListAlloc->Reset());

	ThrowIfFailed(m_CommandList->Reset(cmdListAlloc.Get(), m_PSOs["opaque"]->m_PipelineState.Get()));

	m_CommandList->RSSetViewports(1, &m_Viewport);
	m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

	auto rb1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_CommandList->ResourceBarrier(1, &rb1);

	m_CommandList->ClearRenderTargetView(CurrentBackBufferView(), DirectX::Colors::LightSteelBlue, 0, nullptr);
	m_CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	auto bv = CurrentBackBufferView();
	auto dsv = DepthStencilView();
	m_CommandList->OMSetRenderTargets(1, &bv, true, &dsv);

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_SrvHeap.Get() };
	m_CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	m_CommandList->SetGraphicsRootSignature(m_RootSignatures["default"]->GetRootSignature());

	auto passCB = m_CurrFrameResource->m_PassUniform->Resource();
	m_CommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	auto rb2 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_CommandList->ResourceBarrier(1, &rb2);

	ThrowIfFailed(m_CommandList->Close());

	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	ThrowIfFailed(m_SwapChain->Present(0, 0));
	m_CurrBackBuffer = (m_CurrBackBuffer + 1) % s_SwapChainBufferCount;

	m_CurrFrameResource->m_FenceValue = ++m_CurrentFence;

	FlushCommandQueue();
}

Share<PipelineState> DX12RHI::BuildPSO(Share<Shader> vsShader, Share<Shader> psShader, const V_Array<ShaderLayout>& Layout)
{
	return nullptr;
}

Share<VertexBuffer> DX12RHI::CreateVertexBuffer(float* vertices, uint32_t size, uint32_t elementSize, const VertexBufferLayout& layout)
{
	return MakeShare<DX12VertexBuffer>(m_Device.Get(), m_CommandList.Get(), vertices, size, elementSize, layout);
}

Share<IndexBuffer> DX12RHI::CreateIndexBuffer(uint32_t* indices, uint32_t count, ShaderDataType indiceDataType)
{
	return MakeShare<DX12IndexBuffer>(m_Device.Get(), m_CommandList.Get(), indices, count, indiceDataType);
}

void DX12RHI::Resize(const RHIInitializeParam& param)
{
	FlushCommandQueue();

	ThrowIfFailed(m_CommandList->Reset(m_CommandAllocator.Get(), nullptr));

	for (size_t i = 0; i < s_SwapChainBufferCount; i++)
	{
		m_SwapChainBuffer[i].Reset();
	}
	m_DepthStencilBuffer.Reset();

	ThrowIfFailed(m_SwapChain->ResizeBuffers(s_SwapChainBufferCount, param.WindowWidth, param.WindowHeight,
		m_BackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	m_CurrBackBuffer = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < s_SwapChainBufferCount; i++)
	{
		ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_SwapChainBuffer[i])));
		m_Device->CreateRenderTargetView(m_SwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, m_RtvDescriptorSize);
	}

	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = param.WindowWidth;
	depthStencilDesc.Height = param.WindowHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthStencilDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = m_DepthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	auto heapDefault = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	m_Device->CreateCommittedResource(&heapDefault, D3D12_HEAP_FLAG_NONE, &depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON, &optClear, IID_PPV_ARGS(m_DepthStencilBuffer.GetAddressOf()));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = m_DepthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;
	m_Device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

	auto sb_2w = CD3DX12_RESOURCE_BARRIER::Transition(m_DepthStencilBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	m_CommandList->ResourceBarrier(1, &sb_2w);

	ThrowIfFailed(m_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	FlushCommandQueue();

	m_Viewport.TopLeftX = 0;
	m_Viewport.TopLeftY = 0;
	m_Viewport.Width = static_cast<float>(param.WindowWidth);
	m_Viewport.Height = static_cast<float>(param.WindowHeight);
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.MaxDepth = 1.0f;

	m_ScissorRect = { 0, 0, param.WindowWidth, param.WindowHeight };
}

void DX12RHI::InitializeBase(const RHIInitializeParam& param)
{
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_Factory)));

	EnumAdaptersAndCreateDevice();

	m_RtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_DsvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_CbvSrvUavDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	CreateFence();

	CheckQualityLevel();

	CreateCommandQueue();

	CreateSwapChain();

	CreateRtvAndDsvHeaps();

	Resize(param);
}

void DX12RHI::LoadTextures()
{
	m_Textures["bricksTex"] = MakeUnique<DX12Texture>(m_Device.Get(), m_CommandList.Get(), "bricksTex", DEFAULT_TEXTURE_PATH_1);
	m_Textures["checkboardTex"] = MakeUnique<DX12Texture>(m_Device.Get(), m_CommandList.Get(), "stoneTex", DEFAULT_TEXTURE_PATH_7);
	m_Textures["iceTex"] = MakeUnique<DX12Texture>(m_Device.Get(), m_CommandList.Get(), "tileTex", DEFAULT_TEXTURE_PATH_8);
	m_Textures["white1x1Tex"] = MakeUnique<DX12Texture>(m_Device.Get(), m_CommandList.Get(), "tileTex", DEFAULT_TEXTURE_PATH_9);
}

void DX12RHI::InitBaseMaterials()
{
	m_BaseMaterials["grass"] = MakeUnique<Material>();
	m_BaseMaterials["grass"]->m_Name = "grass";
	m_BaseMaterials["grass"]->m_DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_BaseMaterials["grass"]->m_FresnelR0 = { 0.01f, 0.01f, 0.01f };
	m_BaseMaterials["grass"]->m_Roughness = 0.125f;
	m_BaseMaterials["grass"]->MatCBIndex = 0;
	m_BaseMaterials["grass"]->DiffuseSrvHeapIndex = 0;

	m_BaseMaterials["water"] = MakeUnique<Material>();
	m_BaseMaterials["water"]->m_Name = "water";
	m_BaseMaterials["water"]->m_DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 0.5f };
	m_BaseMaterials["water"]->m_FresnelR0 = { 0.1f, 0.1f, 0.1f };
	m_BaseMaterials["water"]->m_Roughness = 0.0f;
	m_BaseMaterials["water"]->MatCBIndex = 1;
	m_BaseMaterials["water"]->DiffuseSrvHeapIndex = 1;

	m_BaseMaterials["wirefence"] = MakeUnique<Material>();
	m_BaseMaterials["wirefence"]->m_Name = "wirefence";
	m_BaseMaterials["wirefence"]->m_DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_BaseMaterials["wirefence"]->m_FresnelR0 = { 0.1f, 0.1f, 0.1f };
	m_BaseMaterials["wirefence"]->m_Roughness = 0.25f;
	m_BaseMaterials["wirefence"]->MatCBIndex = 2;
	m_BaseMaterials["wirefence"]->DiffuseSrvHeapIndex = 2;
}

void DX12RHI::InitBaseShaders()
{
	const D3D_SHADER_MACRO defines[] =
	{
		"FOG", "1",
		NULL, NULL
	};
	m_Shaders["standardVS"] = MakeShare<DX12Shader>(DEFAULT_SHADER_PATH, nullptr, "VS", "vs_5_0");
	m_Shaders["opaquePS"] = MakeShare<DX12Shader>(DEFAULT_SHADER_PATH, defines, "PS", "ps_5_0");

	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"FOG", "1",
		"ALPHA_TEST", "1",
		NULL, NULL
	};
	m_Shaders["alphaTestedPS"] = MakeShare<DX12Shader>(DEFAULT_SHADER_PATH, alphaTestDefines, "PS", "ps_5_0");
}

void DX12RHI::InitBaseRootSignatures()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_ROOT_PARAMETER slotRootParameter[4];

	slotRootParameter[0].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsConstantBufferView(0);
	slotRootParameter[2].InitAsConstantBufferView(1);
	slotRootParameter[3].InitAsConstantBufferView(2);

	auto staticSamplers = DX12RootSignature::GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter, (UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	m_RootSignatures["default"] = MakeShare<DX12RootSignature>(m_Device.Get(), rootSigDesc);
}

void DX12RHI::InitRenderLayers()
{
	m_RenderLayers[(int)RenderLayer::Opaque].push_back(&m_RenderItems["room"]->m_RenderInstance["floor"]);
}

void DX12RHI::InitBasePSOs()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;
	auto vertexBuffer = dynamic_cast<DX12VertexBuffer*>(m_Vertex_Index_Buffers["landGeo"].first.get());
	{
		ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		opaquePsoDesc.InputLayout = { vertexBuffer->m_VertexLayout.data(), (uint32_t)vertexBuffer->m_VertexLayout.size() };
		opaquePsoDesc.pRootSignature = m_RootSignatures["default"]->GetRootSignature();
		opaquePsoDesc.VS =
		{
			reinterpret_cast<BYTE*>(((ID3D10Blob*)m_Shaders["standardVS"]->GetShader())->GetBufferPointer()),
			((ID3D10Blob*)m_Shaders["standardVS"]->GetShader())->GetBufferSize()
		};
		opaquePsoDesc.PS =
		{
			reinterpret_cast<BYTE*>(((ID3D10Blob*)m_Shaders["opaquePS"]->GetShader())->GetBufferPointer()),
			((ID3D10Blob*)m_Shaders["opaquePS"]->GetShader())->GetBufferSize()
		};
		opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		opaquePsoDesc.SampleMask = UINT_MAX;
		opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		opaquePsoDesc.NumRenderTargets = 1;
		opaquePsoDesc.RTVFormats[0] = m_BackBufferFormat;
		opaquePsoDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
		opaquePsoDesc.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
		opaquePsoDesc.DSVFormat = m_DepthStencilFormat;

		m_PSOs["opaque"] = MakeShare<DX12PipelineState>(m_Device.Get(), opaquePsoDesc);
	}
	
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentPsoDesc = opaquePsoDesc;

		D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
		transparencyBlendDesc.BlendEnable = true;
		transparencyBlendDesc.LogicOpEnable = false;
		transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
		transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
		transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
		transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
		transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		transparentPsoDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
		
		m_PSOs["transparent"] = MakeShare<DX12PipelineState>(m_Device.Get(), transparentPsoDesc);
	}

	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC alphaTestedPsoDesc = opaquePsoDesc;
		/*alphaTestedPsoDesc.PS =
		{
			reinterpret_cast<BYTE*>(((ID3D10Blob*)m_Shaders["alphaTestedPS"]->GetShader())->GetBufferPointer()),
			((ID3D10Blob*)m_Shaders["alphaTestedPS"]->GetShader())->GetBufferSize()
		};
		alphaTestedPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;*/

		m_PSOs["alphaTested"] = MakeShare<DX12PipelineState>(m_Device.Get(), alphaTestedPsoDesc);
	}
	
}

void DX12RHI::EnumAdaptersAndCreateDevice()
{
	D3D_FEATURE_LEVEL emFeatureLevel = D3D_FEATURE_LEVEL_12_1;
	DXGI_ADAPTER_DESC1 stAdapterDesc = {};
	Microsoft::WRL::ComPtr<IDXGIAdapter1> pWarpAdapter;
	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != m_Factory->EnumAdapters1(adapterIndex, &pWarpAdapter); ++adapterIndex)
	{
		pWarpAdapter->GetDesc1(&stAdapterDesc);

		if (stAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{//跳过软件虚拟适配器设备
			continue;
		}
		//检查适配器对D3D支持的兼容级别，这里直接要求支持12.1的能力，注意返回接口的那个参数被置为了nullptr，这样
		//就不会实际创建一个设备了，也不用我们啰嗦的再调用release来释放接口。这也是一个重要的技巧，请记住！
		if (SUCCEEDED(D3D12CreateDevice(pWarpAdapter.Get(), emFeatureLevel, _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	ThrowIfFailed(D3D12CreateDevice(pWarpAdapter.Get(), emFeatureLevel, IID_PPV_ARGS(&m_Device)));
}

void DX12RHI::CreateCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC stQueueDesc = {};
	stQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	stQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(m_Device->CreateCommandQueue(&stQueueDesc, IID_PPV_ARGS(&m_CommandQueue)));

	ThrowIfFailed(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_CommandAllocator.GetAddressOf())));

	ThrowIfFailed(m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(),
		nullptr, IID_PPV_ARGS(m_CommandList.GetAddressOf())));

	m_CommandList->Close();
}

void DX12RHI::CreateSwapChain()
{
	m_SwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = m_InitParam.WindowWidth;
	sd.BufferDesc.Height = m_InitParam.WindowHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = m_BackBufferFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	sd.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaState - 1) : 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = s_SwapChainBufferCount;
	sd.OutputWindow = (HWND)m_InitParam.HWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ThrowIfFailed(m_Factory->CreateSwapChain(m_CommandQueue.Get(), &sd, m_SwapChain.GetAddressOf()));
}

Camera* DX12RHI::GetMainCamera()
{
	return &m_PrespectiveCamera;
}

void DX12RHI::CreateRtvAndDsvHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC stRTVHeapDesc = {};
	stRTVHeapDesc.NumDescriptors = s_SwapChainBufferCount;
	stRTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	stRTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	stRTVHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_Device->CreateDescriptorHeap(&stRTVHeapDesc, IID_PPV_ARGS(m_RtvHeap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_Device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(m_DsvHeap.GetAddressOf())));
}

void DX12RHI::CreateFence()
{
	ThrowIfFailed(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));
}

void DX12RHI::CheckQualityLevel()
{
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = m_BackBufferFormat;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	ThrowIfFailed(m_Device->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)));

	m_4xMsaaQuality = msQualityLevels.NumQualityLevels;
	assert(m_4xMsaaQuality > 0 && "Unexpected MSAA quality level.");
}

void DX12RHI::AnimationMaterials(const RiverTime& time)
{
	auto waterMat = m_BaseMaterials["water"].get();

	float& tu = waterMat->m_MatTransform(3, 0);
	float& tv = waterMat->m_MatTransform(3, 1);

	tu += 0.1f * time.DeltaTime();
	tv += 0.02f * time.DeltaTime();

	if (tu >= 1.0f)
		tu -= 1.0f;

	if (tv >= 1.0f)
		tv -= 1.0f;

	waterMat->m_MatTransform(3, 0) = tu;
	waterMat->m_MatTransform(3, 1) = tv;

	waterMat->NumFramesDirty = 3;
}

void DX12RHI::UpdateObjectCBs()
{
	auto& currObjectCB = m_CurrFrameResource->m_ObjectUniform;
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 texMatrix;
	/*for (auto& e : m_RenderItems)
	{
		if (e->NumFramesDirty > 0)
		{
			memcpy(&worldMatrix, &e->m_WorldTransform, sizeof(worldMatrix));
			memcpy(&texMatrix, &e->m_TexTransform, sizeof(texMatrix));
			DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&worldMatrix);
			DirectX::XMMATRIX texTransform = DirectX::XMLoadFloat4x4(&texMatrix);

			ObjectUniform  objConstants;
			XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(world));
			XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));

			currObjectCB->CopyData(e->m_ObjectIndex, objConstants);

			e->NumFramesDirty--;
		}
	}*/
}

void DX12RHI::UpdateMaterialCBs()
{
	auto currMaterialCB = m_CurrFrameResource->m_MaterialUniform.get();
	for (auto& e : m_BaseMaterials)
	{
		Material* mat = e.second.get();
		if (mat->NumFramesDirty > 0)
		{
			DirectX::XMMATRIX matTransform = DirectX::XMLoadFloat4x4((DirectX::XMFLOAT4X4*)(&mat->m_MatTransform));

			MaterialUniform matConstants;
			matConstants.DiffuseAlbedo = *(DirectX::XMFLOAT4*)(&mat->m_DiffuseAlbedo);
			matConstants.FresnelR0 = *(DirectX::XMFLOAT3*)(&mat->m_FresnelR0);
			matConstants.Roughness = mat->m_Roughness;
			XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

			currMaterialCB->CopyData(mat->MatCBIndex, matConstants);

			mat->NumFramesDirty--;
		}
	}
}

void DX12RHI::UpdateMainPass(const RiverTime& time)
{
	PassUniform mMainPassCB;
	DirectX::XMMATRIX view = m_PrespectiveCamera.GetView();
	DirectX::XMMATRIX proj = m_PrespectiveCamera.GetProj();

	DirectX::XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	auto determinant = XMMatrixDeterminant(view);
	DirectX::XMMATRIX invView = XMMatrixInverse(&determinant, view);

	determinant = XMMatrixDeterminant(proj);
	DirectX::XMMATRIX invProj = XMMatrixInverse(&determinant, proj);

	determinant = XMMatrixDeterminant(viewProj);
	DirectX::XMMATRIX invViewProj = XMMatrixInverse(&determinant, viewProj);

	DirectX::XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
	DirectX::XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
	DirectX::XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
	DirectX::XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
	DirectX::XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	DirectX::XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	mMainPassCB.EyePosW = m_PrespectiveCamera.GetLook();
	mMainPassCB.RenderTargetSize = DirectX::XMFLOAT2((float)m_InitParam.WindowWidth, (float)m_InitParam.WindowHeight);
	mMainPassCB.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / m_InitParam.WindowWidth, 1.0f / m_InitParam.WindowHeight);
	mMainPassCB.NearZ = 1.0f;
	mMainPassCB.FarZ = 1000.0f;
	mMainPassCB.TotalTime = time.TotalTime();
	mMainPassCB.DeltaTime = time.DeltaTime();
	mMainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	mMainPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	mMainPassCB.Lights[0].Strength = { 0.6f, 0.6f, 0.6f };
	mMainPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
	mMainPassCB.Lights[1].Strength = { 0.3f, 0.3f, 0.3f };
	mMainPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
	mMainPassCB.Lights[2].Strength = { 0.15f, 0.15f, 0.15f };

	auto currPassCB = m_CurrFrameResource->m_PassUniform.get();
	currPassCB->CopyData(0, mMainPassCB);
}

void DX12RHI::FlushCommandQueue()
{
	m_CurrentFence++;

	m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence);
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12RHI::CurrentBackBufferView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		m_RtvHeap->GetCPUDescriptorHandleForHeapStart(),
		m_CurrBackBuffer,
		m_RtvDescriptorSize);
}

void DX12RHI::InitDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = (int)m_Textures.size();
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(m_Device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_SrvHeap)));

	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(m_SrvHeap->GetCPUDescriptorHandleForHeapStart());

	auto bricksTex = m_Textures["bricksTex"]->GetResource();
	auto checkboardTex = m_Textures["checkboardTex"]->GetResource();
	auto iceTex = m_Textures["iceTex"]->GetResource();
	auto white1x1Tex = m_Textures["white1x1Tex"]->GetResource();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = bricksTex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	m_Device->CreateShaderResourceView(bricksTex.Get(), &srvDesc, hDescriptor);

	hDescriptor.Offset(1, m_CbvSrvUavDescriptorSize);

	srvDesc.Format = checkboardTex->GetDesc().Format;
	m_Device->CreateShaderResourceView(checkboardTex.Get(), &srvDesc, hDescriptor);

	hDescriptor.Offset(1, m_CbvSrvUavDescriptorSize);

	srvDesc.Format = iceTex->GetDesc().Format;
	m_Device->CreateShaderResourceView(iceTex.Get(), &srvDesc, hDescriptor);

	hDescriptor.Offset(1, m_CbvSrvUavDescriptorSize);

	srvDesc.Format = white1x1Tex->GetDesc().Format;
	m_Device->CreateShaderResourceView(white1x1Tex.Get(), &srvDesc, hDescriptor);
}

void DX12RHI::InitBaseVertexBufferAndIndexBuffer()
{
	VertexBufferLayout layout = { {ShaderDataType::Float3, "POSITION"}, { ShaderDataType::Float3, "NORMAL" }, { ShaderDataType::Float2, "TEXCOORD" } };

	{
		auto roomRenderItem = std::make_unique<RenderItem>();
		std::array<Vertex, 20> vertices =
		{
			// Floor: Observe we tile texture coordinates.
			Vertex(-3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f), // 0 
			Vertex(-3.5f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f),
			Vertex(7.5f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f),
			Vertex(7.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 4.0f, 4.0f),

			// Wall: Observe we tile texture coordinates, and that we
			// leave a gap in the middle for the mirror.
			Vertex(-3.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f), // 4
			Vertex(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
			Vertex(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 0.0f),
			Vertex(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 2.0f),

			Vertex(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f), // 8 
			Vertex(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
			Vertex(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f),
			Vertex(7.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 2.0f),

			Vertex(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f), // 12
			Vertex(-3.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
			Vertex(7.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 0.0f),
			Vertex(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 1.0f),

			// Mirror
			Vertex(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f), // 16
			Vertex(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
			Vertex(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f),
			Vertex(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f)
		};

		std::array<std::int32_t, 30> indices =
		{
			// Floor
			0, 1, 2,
			0, 2, 3,

			// Walls
			4, 5, 6,
			4, 6, 7,

			8, 9, 10,
			8, 10, 11,

			12, 13, 14,
			12, 14, 15,

			// Mirror
			16, 17, 18,
			16, 18, 19
		};

		RenderItemInstance floorSubmesh;
		floorSubmesh.m_WorldTransform = River::Matrix4x4::UnitMatrix();
		floorSubmesh.m_TexTransform = River::Matrix4x4::UnitMatrix();
		floorSubmesh.m_ObjectIndex = 0;
		floorSubmesh.m_Material = m_BaseMaterials["checkertile"].get();
		floorSubmesh.m_PrimitiveType = PrimitiveType::TriangleList;
		floorSubmesh.IndexCount = 6;
		floorSubmesh.StartIndexLocation = 0;
		floorSubmesh.BaseVertexLocation = 0;
		roomRenderItem->m_RenderInstance["floor"] = floorSubmesh;

		RenderItemInstance wallSubmesh;
		wallSubmesh.m_WorldTransform = River::Matrix4x4::UnitMatrix();
		wallSubmesh.m_TexTransform = River::Matrix4x4::UnitMatrix();
		wallSubmesh.m_ObjectIndex = 0;
		wallSubmesh.m_Material = m_BaseMaterials["checkertile"].get();
		wallSubmesh.m_PrimitiveType = PrimitiveType::TriangleList;
		wallSubmesh.IndexCount = 18;
		wallSubmesh.StartIndexLocation = 6;
		wallSubmesh.BaseVertexLocation = 0;
		roomRenderItem->m_RenderInstance["wall"] = wallSubmesh;

		RenderItemInstance mirrorSubmesh;
		mirrorSubmesh.m_WorldTransform = River::Matrix4x4::UnitMatrix();
		mirrorSubmesh.m_TexTransform = River::Matrix4x4::UnitMatrix();
		mirrorSubmesh.m_ObjectIndex = 0;
		mirrorSubmesh.m_Material = m_BaseMaterials["checkertile"].get();
		mirrorSubmesh.m_PrimitiveType = PrimitiveType::TriangleList;
		mirrorSubmesh.IndexCount = 6;
		mirrorSubmesh.StartIndexLocation = 24;
		mirrorSubmesh.BaseVertexLocation = 0;
		roomRenderItem->m_RenderInstance["mirror"] = mirrorSubmesh;

		m_Vertex_Index_Buffers["room"] = {
			CreateVertexBuffer((float*)vertices.data(), (UINT)(vertices.size() * sizeof(Vertex)), (UINT)sizeof(Vertex), layout),
			CreateIndexBuffer((UINT*)indices.data(), (UINT)indices.size(), ShaderDataType::Int)
		};
		roomRenderItem->m_VertexBuffer = m_Vertex_Index_Buffers["room"].first.get();
		roomRenderItem->m_IndexBuffer = m_Vertex_Index_Buffers["room"].second.get();

		m_RenderItems["room"] = std::move(roomRenderItem);
	}
	{
		
	}

	{
		auto& meshData = DX12GeometryGenerator::Get()->GetMeshData(GeometryType::Box);
		V_Array<Vertex> vertices(meshData.Vertices.size());
		for (size_t i = 0; i < vertices.size(); i++)
		{
			vertices[i].Pos = meshData.Vertices[i].Position;
			vertices[i].Normal = meshData.Vertices[i].Normal;
			vertices[i].TexC = meshData.Vertices[i].TexC;
		}

		m_Vertex_Index_Buffers["boxGeo"] = {
			RHI::Get()->CreateVertexBuffer((float*)vertices.data(), (unsigned int)(vertices.size() * sizeof(Vertex)), (uint32_t)sizeof(Vertex), layout),
			MakeShare<DX12IndexBuffer>(m_Device.Get(), m_CommandList.Get(),(uint32_t*)meshData.Indices32.data(), (uint32_t)meshData.Indices32.size(), ShaderDataType::Int)
		};
	}
}

void DX12RHI::InitFrameBuffer()
{
	for (int i = 0; i < s_FrameBufferCount; ++i)
	{
		m_FrameBuffer.push_back(MakeUnique<DX12FrameBuffer>(m_Device.Get(), 1, s_MaxRenderItem, 
			(int)m_BaseMaterials.size(), 1));
	}
}

void DX12RHI::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const V_Array<RenderItem*>& items)
{
	UINT objCBByteSize = RendererUtil::CalcMinimumGPUAllocSize(sizeof(ObjectUniform));
	UINT matCBByteSize = RendererUtil::CalcMinimumGPUAllocSize(sizeof(MaterialUniform));

	auto objectCB = m_CurrFrameResource->m_ObjectUniform->Resource();
	auto matCB = m_CurrFrameResource->m_MaterialUniform->Resource();

	// For each render item...
	DX12VertexBuffer* vertexBuffer = nullptr;
	DX12IndexBuffer* indexBuffer = nullptr;
	for (size_t i = 0; i < items.size(); ++i)
	{
		auto ri = items[i];

		vertexBuffer = dynamic_cast<DX12VertexBuffer*>(ri->m_VertexBuffer);
		indexBuffer = dynamic_cast<DX12IndexBuffer*>(ri->m_IndexBuffer);

		
		cmdList->IASetVertexBuffers(0, 1, &vertexBuffer->m_VertexBufferView);

		cmdList->IASetIndexBuffer(&indexBuffer->m_IndexBufferView);
		cmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(m_SrvHeap->GetGPUDescriptorHandleForHeapStart());
		//tex.Offset(ri->m_Material->DiffuseSrvHeapIndex, m_CbvSrvUavDescriptorSize);

		//D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->m_ObjectIndex * objCBByteSize;
		//D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->m_Material->MatCBIndex * matCBByteSize;

		cmdList->SetGraphicsRootDescriptorTable(0, tex);
		//cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
		//cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

		cmdList->DrawIndexedInstanced(indexBuffer->GetIndexCount(), 1,
			0, 0, 0);
	}
}
