#include "RiverPch.h"
#include "RendererUtil.h"
#include "Renderer/DX12Renderer/Header/DX12Util.h"
#include "Renderer/DX12Renderer/Header/d3dx12.h"
#include "Renderer/DX12Renderer/Header/DX12RHI.h"
#include "Renderer/DX12Renderer/Header/DX12PipelineState.h"
#include "Renderer/DX12Renderer/Header/DX12VertexBuffer.h"
#include "Renderer/DX12Renderer/Header/DX12IndexBuffer.h"
#include "Renderer/DX12Renderer/Header/DX12Shader.h"
#include "Renderer/DX12Renderer/Header/DX12GeometryGenerator.h"
#include "Renderer/DX12Renderer/Header/DX12Texture.h"

#include "DirectXMath.h"
#include <d3dcompiler.h>
#include <iostream>
#include <chrono>

#define DEFAULT_SHADER_PATH "F:\\GitHub\\River\\River\\Shaders\\color.hlsl"
#define DEFAULT_TEXTURE_PATH_1 "F:\\GitHub\\River\\River\\Textures\\bricks.dds"
#define DEFAULT_TEXTURE_PATH_2 "F:\\GitHub\\River\\River\\Textures\\stone.dds"
#define DEFAULT_TEXTURE_PATH_3 "F:\\GitHub\\River\\River\\Textures\\tile.dds"

struct Vertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT3 Normal;
};

DirectX::XMFLOAT4X4 mWorld = Identity4x4();

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

Share<VertexBuffer> mVertexBuffer;
Share<IndexBuffer> mIndexBuffer;

void DX12RHI::Initialize(const RHIInitializeParam& param)
{
	m_InitParam = param;

	m_PrespectiveCamera.SetPosition(0.0f, 0.0f, -5.0f);

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

	ThrowIfFailed(m_CommandList->Reset(m_CommandAllocator.Get(), nullptr));

	{
		//BuildConstantBuffers();
		InitFrameBuffer();
		BuildDescriptorHeaps();
		BuildConstantBufferViews();

		m_Shader = MakeShare<DX12Shader>(DEFAULT_SHADER_PATH);
		BuildTestVertexBufferAndIndexBuffer();

		m_PSOs.push_back(MakeShare<DX12PipelineState>(m_Device.Get(), m_Shader, mVertexBuffer));
	}

	ThrowIfFailed(m_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	FlushCommandQueue();
}

void DX12RHI::OnUpdate()
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

	ObjectUniform objConstants;

	auto currObjectCB = m_CurrFrameResource->m_ObjectUniform.get();
	for (int i = 0; i < s_MaxRenderItem; i++)
	{
		DirectX::XMMATRIX world = DirectX::XMMatrixTranslation(1 + 2.0f * i, 1.5f * 2 * i, i * 10.0f);
		//DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&mWorld);
		DirectX::XMMATRIX worldViewProj = world * m_PrespectiveCamera.GetView() * m_PrespectiveCamera.GetProj();
		DirectX::XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(worldViewProj));

		currObjectCB->CopyData(i, objConstants);
	}

	UpdateMainPass();
}

void DX12RHI::Render()
{
	auto pso = m_PSOs[0]->m_PipelineState.Get();
	auto rootSignature = m_PSOs[0]->m_RootSignature.Get();

	auto cmdListAlloc = m_CurrFrameResource->m_CommandAlloc;

	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(cmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	ThrowIfFailed(m_CommandList->Reset(cmdListAlloc.Get(), pso));

	m_CommandList->RSSetViewports(1, &m_Viewport);
	m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

	// Indicate a state transition on the resource usage.
	auto rb1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_CommandList->ResourceBarrier(1, &rb1);

	// Clear the back buffer and depth buffer.
	m_CommandList->ClearRenderTargetView(CurrentBackBufferView(), DirectX::Colors::LightSteelBlue, 0, nullptr);
	m_CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	auto bv = CurrentBackBufferView();
	auto dsv = DepthStencilView();
	m_CommandList->OMSetRenderTargets(1, &bv, true, &dsv);

	ID3D12DescriptorHeap* descriptorHeaps[] = { /*mUniformBuffer->m_UniformBufferHeap.Get()*/ m_CbvHeap.Get() };
	m_CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	m_CommandList->SetGraphicsRootSignature(rootSignature);

	int passCbvIndex = s_MaxRenderItem * s_FrameBufferCount + m_CurrFrameResourceIndex;
	auto passCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_CbvHeap->GetGPUDescriptorHandleForHeapStart());
	passCbvHandle.Offset(passCbvIndex, m_CbvSrvUavDescriptorSize);
	m_CommandList->SetGraphicsRootDescriptorTable(1, passCbvHandle);

	int MaterialCbvIndex = s_MaxRenderItem * s_FrameBufferCount + s_FrameBufferCount + m_CurrFrameResourceIndex;
	auto materialCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_CbvHeap->GetGPUDescriptorHandleForHeapStart());
	materialCbvHandle.Offset(MaterialCbvIndex, m_CbvSrvUavDescriptorSize);
	m_CommandList->SetGraphicsRootDescriptorTable(2, materialCbvHandle);

	auto bbv = dynamic_cast<DX12VertexBuffer*>(mVertexBuffer.get())->m_VertexBufferView;
	auto ibv = dynamic_cast<DX12IndexBuffer*>(mIndexBuffer.get())->m_IndexBufferView;

	for (int i = 0; i < s_MaxRenderItem; i++)
	{
		m_CommandList->IASetVertexBuffers(0, 1, &bbv);
		m_CommandList->IASetIndexBuffer(&ibv);
		m_CommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(m_SrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		tex.Offset( i % m_Textures.size(), m_CbvSrvUavDescriptorSize);

		UINT cbvIndex = m_CurrFrameResourceIndex * s_MaxRenderItem + i;
		auto cbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_CbvHeap->GetGPUDescriptorHandleForHeapStart());
		cbvHandle.Offset(cbvIndex, m_CbvSrvUavDescriptorSize);

		m_CommandList->SetGraphicsRootDescriptorTable(3, tex);
		m_CommandList->SetGraphicsRootDescriptorTable(0, /*m_CbvHeap->GetGPUDescriptorHandleForHeapStart()*/cbvHandle);
		m_CommandList->DrawIndexedInstanced(
			mIndexBuffer->GetIndexCount(),//m_BoxGeo->DrawArgs["box"].IndexCount,
			1, 0, 0, 0);
	}

	// Indicate a state transition on the resource usage.
	auto rb2 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_CommandList->ResourceBarrier(1, &rb2);

	// Done recording commands.
	ThrowIfFailed(m_CommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// swap the back and front buffers
	ThrowIfFailed(m_SwapChain->Present(0, 0));
	m_CurrBackBuffer = (m_CurrBackBuffer + 1) % s_SwapChainBufferCount;

	m_CurrFrameResource->m_FenceValue = ++m_CurrentFence;

	// Wait until frame commands are complete.  This waiting is inefficient and is
	// done for simplicity.  Later we will show how to organize our rendering code
	// so we do not have to wait per frame.
	FlushCommandQueue();
}

Share<PipelineState> DX12RHI::BuildPSO(Share<Shader> Shader, const V_Array<ShaderLayout>& Layout)
{
	return MakeShare<DX12PipelineState>(m_Device.Get(), Shader, nullptr);
}

Share<VertexBuffer> DX12RHI::CreateVertexBuffer(float* vertices, uint32_t size, uint32_t elementSize, const VertexBufferLayout& layout)
{
	return MakeShare<DX12VertexBuffer>(m_Device.Get(), vertices, size, elementSize, layout);
}

Share<IndexBuffer> DX12RHI::CreateIndexBuffer(uint32_t* indices, uint32_t count, ShaderDataType indiceDataType)
{
	return MakeShare<DX12IndexBuffer>(m_Device.Get(), indices, count, indiceDataType);
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

void DX12RHI::LoadTextures()
{
	m_Textures.push_back(MakeUnique<DX12Texture>(m_Device.Get(), m_CommandList.Get(), "bricksTex", DEFAULT_TEXTURE_PATH_1));
	m_Textures.push_back(MakeUnique<DX12Texture>(m_Device.Get(), m_CommandList.Get(), "stoneTex", DEFAULT_TEXTURE_PATH_2));
	m_Textures.push_back(MakeUnique<DX12Texture>(m_Device.Get(), m_CommandList.Get(), "tileTex", DEFAULT_TEXTURE_PATH_3));
}

void DX12RHI::InitBaseMaterials()
{
	m_BaseMaterials.resize(s_BaseMaterialCount);

	m_BaseMaterials[0].m_Name = "bricks0";
	m_BaseMaterials[0].m_DiffuseAlbedo = { 0.133333340f, 0.545098066f, 0.133333340f, 1.000000000f };
	m_BaseMaterials[0].m_FresnelR0 = { 0.02f, 0.02f, 0.02f };
	m_BaseMaterials[0].m_Roughness = 0.1f;

	/*m_BaseMaterials[1].m_Name = "stone0";
	m_BaseMaterials[1].m_DiffuseAlbedo = { 0.690196097f, 0.768627524f, 0.870588303f, 1.000000000f };
	m_BaseMaterials[1].m_FresnelR0 = { 0.0f, 1.f, 0.f };
	m_BaseMaterials[1].m_Roughness = 0.3f;

	m_BaseMaterials[2].m_Name = "tile0";
	m_BaseMaterials[2].m_DiffuseAlbedo = { 0.827451050f, 0.827451050f, 0.827451050f, 1.000000000f };
	m_BaseMaterials[2].m_FresnelR0 = { 0.f, 0.f, 1.f };
	m_BaseMaterials[2].m_Roughness = 0.8f;*/
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

void DX12RHI::UpdateMainPass()
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
	mMainPassCB.TotalTime = 0.f;// gt.TotalTime();
	mMainPassCB.DeltaTime = 0.f;//gt.DeltaTime();
	mMainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	mMainPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	mMainPassCB.Lights[0].Strength = { 10.f, 10.f, 10.f };
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

void DX12RHI::BuildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = (s_MaxRenderItem + 1 + 1) * 3; //1;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_Device->CreateDescriptorHeap(&cbvHeapDesc,
		IID_PPV_ARGS(&m_CbvHeap)));

	//
	// Create the SRV heap.
	//
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 3;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(m_Device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_SrvDescriptorHeap)));

	//
	// Fill out the heap with actual descriptors.
	//
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(m_SrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	auto bricksTex = m_Textures[0]->GetResource();
	auto stoneTex = m_Textures[1]->GetResource();
	auto tileTex = m_Textures[2]->GetResource();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = bricksTex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = bricksTex->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	m_Device->CreateShaderResourceView(bricksTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, m_CbvSrvUavDescriptorSize);

	srvDesc.Format = stoneTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = stoneTex->GetDesc().MipLevels;
	m_Device->CreateShaderResourceView(stoneTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, m_CbvSrvUavDescriptorSize);

	srvDesc.Format = tileTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = tileTex->GetDesc().MipLevels;
	m_Device->CreateShaderResourceView(tileTex.Get(), &srvDesc, hDescriptor);
}

void DX12RHI::BuildTestVertexBufferAndIndexBuffer()
{
	auto& meshData = DX12GeometryGenerator::Get()->GetMeshData(GeometryType::Box);
	V_Array<Vertex> vertices(meshData.Vertices.size());
	for (size_t i = 0; i < vertices.size(); i++)
	{
		vertices[i].Pos = meshData.Vertices[i].Position;
		vertices[i].Normal = meshData.Vertices[i].Normal;
	}

	VertexBufferLayout layout = { {ShaderDataType::Float3, "POSITION"}, { ShaderDataType::Float3, "NORMAL" } };

	mVertexBuffer = RHI::Get()->CreateVertexBuffer((float*)vertices.data(), (unsigned int)(vertices.size() * sizeof(Vertex)), (uint32_t)sizeof(Vertex), layout);
	mIndexBuffer = RHI::Get()->CreateIndexBuffer((uint32_t*)meshData.Indices32.data(), (uint32_t)meshData.Indices32.size(), ShaderDataType::Int);
}

void DX12RHI::InitFrameBuffer()
{
	for (int i = 0; i < s_FrameBufferCount; ++i)
	{
		m_FrameBuffer.push_back(MakeUnique<DX12FrameBuffer>(m_Device.Get(), 1, s_MaxRenderItem, s_BaseMaterialCount));
	}

	InitBaseMaterials();
	
	MaterialUniform uniform;
	
	for (size_t j = 0; j < s_BaseMaterialCount; j++)
	{
		memcpy(&uniform.DiffuseAlbedo, &m_BaseMaterials[j].m_DiffuseAlbedo, sizeof(uniform.DiffuseAlbedo));
		memcpy(&uniform.FresnelR0, &m_BaseMaterials[j].m_FresnelR0, sizeof(uniform.FresnelR0));
		uniform.Roughness = m_BaseMaterials[j].m_Roughness;
			
		for (size_t i = 0; i < s_FrameBufferCount; i++)
		{
			m_FrameBuffer[i]->m_MaterialUniform->CopyData(0, uniform);
		}

	}
}

void DX12RHI::BuildConstantBufferViews()
{
	UINT objCBByteSize = RendererUtil::CalcMinimumGPUAllocSize(sizeof(ObjectUniform));

	for (int i = 0; i < m_FrameBuffer.size(); i++)
	{
		auto resource = m_FrameBuffer[i]->m_ObjectUniform->Resource();
		for (int j = 0; j < s_MaxRenderItem; j++)
		{
			D3D12_GPU_VIRTUAL_ADDRESS cbAddress = resource->GetGPUVirtualAddress();
			cbAddress += j * objCBByteSize;

			int heapIndex = i * s_MaxRenderItem + j;
			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_CbvHeap->GetCPUDescriptorHandleForHeapStart());
			handle.Offset(heapIndex, m_CbvSrvUavDescriptorSize);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = cbAddress;
			cbvDesc.SizeInBytes = objCBByteSize;

			m_Device->CreateConstantBufferView(&cbvDesc, handle);
		}
	}

	int passCbvOffset = s_MaxRenderItem * (int)m_FrameBuffer.size();
	UINT passCBByteSize = RendererUtil::CalcMinimumGPUAllocSize(sizeof(PassUniform));

	// Last three descriptors are the pass CBVs for each frame resource.
	for (int frameIndex = 0; frameIndex < m_FrameBuffer.size(); ++frameIndex)
	{
		auto passCB = m_FrameBuffer[frameIndex]->m_PassUniform->Resource();
		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = passCB->GetGPUVirtualAddress();

		// Offset to the pass cbv in the descriptor heap.
		int heapIndex = passCbvOffset + frameIndex;
		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_CbvHeap->GetCPUDescriptorHandleForHeapStart());
		handle.Offset(heapIndex, m_CbvSrvUavDescriptorSize);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = passCBByteSize;

		m_Device->CreateConstantBufferView(&cbvDesc, handle);
	}

	int materialCbvOffset = s_MaxRenderItem * (int)m_FrameBuffer.size() + (int)m_FrameBuffer.size();
	UINT materialCBByteSize = RendererUtil::CalcMinimumGPUAllocSize(sizeof(MaterialUniform));
	for (int frameIndex = 0; frameIndex < m_FrameBuffer.size(); ++frameIndex)
	{
		auto materialCB = m_FrameBuffer[frameIndex]->m_MaterialUniform->Resource();
		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = materialCB->GetGPUVirtualAddress();

		// Offset to the pass cbv in the descriptor heap.
		int heapIndex = materialCbvOffset + frameIndex;
		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_CbvHeap->GetCPUDescriptorHandleForHeapStart());
		handle.Offset(heapIndex, m_CbvSrvUavDescriptorSize);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = materialCBByteSize;

		m_Device->CreateConstantBufferView(&cbvDesc, handle);
	}
}