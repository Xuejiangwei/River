#include "RiverPch.h"
#include "RendererUtil.h"
#include "Renderer/DX12Renderer/Header/DX12Util.h"
#include "Renderer/DX12Renderer/Header/d3dx12.h"
#include "Renderer/DX12Renderer/Header/DX12RHI.h"
#include "Renderer/DX12Renderer/Header/DX12PipelineState.h"
#include "Renderer/DX12Renderer/Header/DX12UniformBuffer.h"
#include "Renderer/DX12Renderer/Header/DX12MeshGeometry.h"
#include "Renderer/DX12Renderer/Header/DX12VertexBuffer.h"
#include "Renderer/DX12Renderer/Header/DX12FrameBuffer.h"
#include "Renderer/DX12Renderer/Header/DX12Shader.h"

#include "DirectXMath.h"
#include <d3dcompiler.h>
#include <iostream>
#include <chrono>

PassUniform g_MainPassCB;

struct Vertex
{
	DirectX::XMFLOAT4 Pos;
	DirectX::XMFLOAT4 Color;
};

DirectX::XMFLOAT4X4 mProj = DirectX::XMFLOAT4X4(
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f);

DirectX::XMFLOAT4X4 mView = DirectX::XMFLOAT4X4(
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f);

DirectX::XMFLOAT4X4 mWorld = DirectX::XMFLOAT4X4(
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f);

struct GRS_VERTEX
{
	DirectX::XMFLOAT4 m_vtPos;
	DirectX::XMFLOAT4 m_vtColor;
};

DX12RHI::DX12RHI() 
	: m_CurrentFence(1), m_CurrBackBuffer(0), m_RtvDescriptorSize(0), m_DsvDescriptorSize(0), m_CbvSrvUavDescriptorSize(0)
{
}

DX12RHI::~DX12RHI()
{
}

Share<DX12VertexBuffer> mVertexBuffer;
Share<DX12UniformBuffer> mUniformBuffer;

void DX12RHI::Initialize(const RHIInitializeParam& param)
{
	m_InitParam = param;

	m_Viewport = { 0.0f, 0.0f, static_cast<float>(param.WindowWidth), static_cast<float>(param.WindowHeight), D3D12_MIN_DEPTH, D3D12_MAX_DEPTH };
	m_ScissorRect = { 0, 0, static_cast<LONG>(param.WindowWidth), static_cast<LONG>(param.WindowHeight) };

	{
		ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_Factory)));

		EnumAdaptersAndCreateDevice();

		CreateFence();

		//得到每个描述符元素的大小
		m_RtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_DsvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_CbvSrvUavDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		CheckQualityLevel();

		CreateCommandQueue();

		CreateSwapChain();

		CreateRtvAndDsvHeaps();

		Resize(param);
	}

	ThrowIfFailed(m_CommandList->Reset(m_CommandAllocator.Get(), nullptr));

	mUniformBuffer = MakeUnique<DX12UniformBuffer>(m_Device.Get(), sizeof(ObjectUnifrom), 1, true);

	{
		// 定义三角形的3D数据结构，每个顶点使用三原色之一
		float fTrangleSize = 3.0f;
		GRS_VERTEX stTriangleVertices[] =
		{
			{ { 0.0f, 0.25f * fTrangleSize, 0.0f ,1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
			{ { 0.25f * fTrangleSize, -0.25f * fTrangleSize, 0.0f ,1.0f  }, { 0.0f, 1.0f, 0.0f, 1.0f } },
			{ { -0.25f * fTrangleSize, -0.25f * fTrangleSize, 0.0f  ,1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
		};

		mVertexBuffer = MakeShare<DX12VertexBuffer>(m_Device.Get(), (float*)(&stTriangleVertices),
			(uint32_t)sizeof(stTriangleVertices), (uint32_t)sizeof(GRS_VERTEX));
	}

	/*for (size_t i = 0; i < s_SwapChainBufferCount; i++)
	{
		m_FrameBuffers[i] = MakeUnique<DX12FrameBuffer>(m_Device.Get(), 1, 1);
	}*/

	auto Shader = MakeShare<DX12Shader>("F:\\GitHub\\River\\River\\Shaders\\shaders.hlsl");
	m_PSOs.push_back(MakeShare<DX12PipelineState>(m_Device.Get(), Shader));

	ThrowIfFailed(m_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	FlushCommandQueue();
}

void DX12RHI::OnUpdate()
{
	//auto& CurrFrameBuffer = m_FrameBuffers[0];
	//std::cout << m_CurrentFence << " " << CurrFrameBuffer->m_FenceValue << std::endl;
	//if (CurrFrameBuffer->m_FenceValue != 0 && m_Fence->GetCompletedValue() < m_CurrentFence)
	//{
	//	HANDLE eventHandle = CreateEvent(nullptr, false, false, nullptr);
	//	ThrowIfFailed(m_Fence->SetEventOnCompletion(m_CurrentFence, eventHandle));
	//	
	//	auto pre = std::chrono::system_clock::now();
	//	WaitForSingleObject(eventHandle, 2000);// INFINITE);

	//	CloseHandle(eventHandle);

	//	auto now = std::chrono::system_clock::now();
	//	std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(now - pre);
	//	std::cout << "time " << time_span.count() << std::endl;
	//}

	ObjectUnifrom objConstants;

	XMStoreFloat4(&objConstants.World, DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f));
	//CurrFrameBuffer->ObjectCB->CopyData(0, &objConstants, sizeof(objConstants));
	mUniformBuffer->CopyData(0, &objConstants, sizeof(objConstants));
}

void DX12RHI::Render()
{
	ThrowIfFailed(m_CommandAllocator->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	ThrowIfFailed(m_CommandList->Reset(m_CommandAllocator.Get(), m_PSOs[0]->m_PipelineState.Get()));

	m_CommandList->RSSetViewports(1, &m_Viewport);
	m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

	// Indicate a state transition on the resource usage.
	auto rb1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_CommandList->ResourceBarrier(1, &rb1);

	// Clear the back buffer and depth buffer.
	m_CommandList->ClearRenderTargetView(CurrentBackBufferView(), DirectX::Colors::LightSteelBlue, 0, nullptr);
	m_CommandList->ClearDepthStencilView(m_DsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	auto vv = m_DsvHeap->GetCPUDescriptorHandleForHeapStart();
	auto bv = CurrentBackBufferView();
	m_CommandList->OMSetRenderTargets(1, &bv, true, &vv);

	ID3D12DescriptorHeap* descriptorHeaps[] = { mUniformBuffer->m_UniformBufferHeap.Get() };
	m_CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	m_CommandList->SetGraphicsRootSignature(m_PSOs[0]->m_RootSignature.Get());

	m_CommandList->IASetVertexBuffers(0, 1, &mVertexBuffer->mVertexBufferView);
	m_CommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_CommandList->SetGraphicsRootDescriptorTable(0, mUniformBuffer->m_UniformBufferHeap->GetGPUDescriptorHandleForHeapStart());

	m_CommandList->DrawInstanced(3, 1, 0, 0);

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
	ThrowIfFailed(m_SwapChain1->Present(0, 0));
	m_CurrBackBuffer = (m_CurrBackBuffer + 1) % s_SwapChainBufferCount;

	// Wait until frame commands are complete.  This waiting is inefficient and is
	// done for simplicity.  Later we will show how to organize our rendering code
	// so we do not have to wait per frame.
	FlushCommandQueue();
}


Share<PipelineState> DX12RHI::BuildPSO(Share<Shader> Shader, const Vector<ShaderLayout>& Layout)
{
	return MakeShare<DX12PipelineState>(m_Device.Get(), Shader);
}

Share<VertexBuffer> DX12RHI::CreateVertexBuffer(float* vertices, uint32_t size, const VertexBufferLayout& layout)
{
	return MakeShare<DX12VertexBuffer>(m_Device.Get(), vertices, size, 0);
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

	ThrowIfFailed(m_SwapChain1->ResizeBuffers(s_SwapChainBufferCount, param.WindowWidth, param.WindowHeight, 
		DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
	m_CurrBackBuffer = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < s_SwapChainBufferCount; i++)
	{
		ThrowIfFailed(m_SwapChain1->GetBuffer(i, IID_PPV_ARGS(&m_SwapChainBuffer[i])));
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
	auto heapDefault = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	m_Device->CreateCommittedResource(&heapDefault, D3D12_HEAP_FLAG_NONE, &depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON, &optClear, IID_PPV_ARGS(m_DepthStencilBuffer.GetAddressOf()));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.Texture2D.MipSlice = 0;
	m_Device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), &dsvDesc, m_DsvHeap->GetCPUDescriptorHandleForHeapStart());

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
	DXGI_SWAP_CHAIN_DESC1 stSwapChainDesc = {};
	stSwapChainDesc.BufferCount = s_SwapChainBufferCount;
	stSwapChainDesc.Width = m_InitParam.WindowWidth;
	stSwapChainDesc.Height = m_InitParam.WindowHeight;
	stSwapChainDesc.Format = m_RenderTargetFormat;
	stSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	stSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	stSwapChainDesc.SampleDesc.Count = 1;

	ThrowIfFailed(m_Factory->CreateSwapChainForHwnd(
		m_CommandQueue.Get(),		// 交换链需要命令队列，Present命令要执行
		(HWND)m_InitParam.HWnd,
		&stSwapChainDesc,
		nullptr,
		nullptr,
		&m_SwapChain1
	));

	ThrowIfFailed(m_SwapChain1.As(&m_SwapChain3));
	//得到当前后缓冲区的序号，也就是下一个将要呈送显示的缓冲区的序号
	//注意此处使用了高版本的SwapChain接口的函数
	m_CurrBackBuffer = m_SwapChain3->GetCurrentBackBufferIndex();
}

void DX12RHI::CreateRtvAndDsvHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC stRTVHeapDesc = {};
	stRTVHeapDesc.NumDescriptors = s_SwapChainBufferCount;
	stRTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	stRTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	ThrowIfFailed(m_Device->CreateDescriptorHeap(&stRTVHeapDesc, IID_PPV_ARGS(&m_RtvHeap)));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_Device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(m_DsvHeap.GetAddressOf())));

	/*D3D12_CPU_DESCRIPTOR_HANDLE stRTVHandle = m_RtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < s_SwapChainBufferCount; i++)
	{
		ThrowIfFailed(m_SwapChain3->GetBuffer(i, IID_PPV_ARGS(&m_SwapChainBuffer[i])));

		m_Device->CreateRenderTargetView(m_SwapChainBuffer[i].Get(), nullptr, stRTVHandle);

		stRTVHandle.ptr += m_RtvDescriptorSize;
	}*/
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
	//DirectX::XMMATRIX view = XMLoadFloat4x4(&mView);
	//DirectX::XMMATRIX proj = XMLoadFloat4x4(&mProj);

	//DirectX::XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	//DirectX::XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	//DirectX::XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	//DirectX::XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	//XMStoreFloat4x4(&g_MainPassCB.View, XMMatrixTranspose(view));
	//XMStoreFloat4x4(&g_MainPassCB.InvView, XMMatrixTranspose(invView));
	//XMStoreFloat4x4(&g_MainPassCB.Proj, XMMatrixTranspose(proj));
	//XMStoreFloat4x4(&g_MainPassCB.InvProj, XMMatrixTranspose(invProj));
	//XMStoreFloat4x4(&g_MainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	//XMStoreFloat4x4(&g_MainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	//g_MainPassCB.EyePosW = { 0.0f, 0.0f, 0.0f };// mEyePos;
	//g_MainPassCB.RenderTargetSize = DirectX::XMFLOAT2((float)m_InitParam.WindowWidth, (float)m_InitParam.WindowHeight);
	//g_MainPassCB.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / m_InitParam.WindowWidth, 1.0f / m_InitParam.WindowHeight);
	//g_MainPassCB.NearZ = 1.0f;
	//g_MainPassCB.FarZ = 1000.0f;
	///*g_MainPassCB.TotalTime = gt.TotalTime();
	//g_MainPassCB.DeltaTime = gt.DeltaTime();*/

	//auto currPassCB = mCurrFrameResource->PassCB.get();
	//currPassCB->CopyData(0, g_MainPassCB);
}

void DX12RHI::FlushCommandQueue()
{
	m_CurrentFence++;

	m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence);

	if (m_Fence->GetCompletedValue() < m_CurrentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

		ThrowIfFailed(m_Fence->SetEventOnCompletion(m_CurrentFence, eventHandle));

		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12RHI::CurrentBackBufferView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		m_RtvHeap->GetCPUDescriptorHandleForHeapStart(),
		m_CurrBackBuffer,
		m_RtvDescriptorSize);
}

void DX12RHI::BuildMeshGeometry(const String& MeshName)
{
	std::array<Vertex, 8> vertices =
	{
		Vertex({ DirectX::XMFLOAT4(0.0f, 0.25f * 3, 0.0f ,1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f ) }),
		Vertex({ DirectX::XMFLOAT4(0.25f * 3, -0.25f * 3, 0.0f ,1.0f), DirectX::XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ) }),
		Vertex({ DirectX::XMFLOAT4(-0.25f * 3, -0.25f * 3, 0.0f  ,1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f ) })
		/*Vertex({ DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT4(DirectX::Colors::White) }),
		Vertex({ DirectX::XMFLOAT3(-1.0f, +1.0f, -1.0f), DirectX::XMFLOAT4(DirectX::Colors::Black) }),
		Vertex({ DirectX::XMFLOAT3(+1.0f, +1.0f, -1.0f), DirectX::XMFLOAT4(DirectX::Colors::Red) }),
		Vertex({ DirectX::XMFLOAT3(+1.0f, -1.0f, -1.0f), DirectX::XMFLOAT4(DirectX::Colors::Green) }),
		Vertex({ DirectX::XMFLOAT3(-1.0f, -1.0f, +1.0f), DirectX::XMFLOAT4(DirectX::Colors::Blue) }),
		Vertex({ DirectX::XMFLOAT3(-1.0f, +1.0f, +1.0f), DirectX::XMFLOAT4(DirectX::Colors::Yellow) }),
		Vertex({ DirectX::XMFLOAT3(+1.0f, +1.0f, +1.0f), DirectX::XMFLOAT4(DirectX::Colors::Cyan) }),
		Vertex({ DirectX::XMFLOAT3(+1.0f, -1.0f, +1.0f), DirectX::XMFLOAT4(DirectX::Colors::Magenta) })*/
	};

	std::array<std::uint16_t, 36> indices =
	{
		// front face
		0, 1, 2,
		//0, 2, 3,

		//// back face
		//4, 6, 5,
		//4, 7, 6,

		//// left face
		//4, 5, 1,
		//4, 1, 0,

		//// right face
		//3, 2, 6,
		//3, 6, 7,

		//// top face
		//1, 5, 6,
		//1, 6, 2,

		//// bottom face
		//4, 0, 3,
		//4, 3, 7
	};

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	m_BoxGeo = MakeUnique<DX12MeshGeometry>();
	m_BoxGeo->Name = MeshName;

	D3DCreateBlob(vbByteSize, &m_BoxGeo->VertexBufferCPU);
	CopyMemory(m_BoxGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	D3DCreateBlob(ibByteSize, &m_BoxGeo->IndexBufferCPU);
	CopyMemory(m_BoxGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	m_BoxGeo->VertexBufferGPU = CreateDefaultBuffer(m_Device.Get(),
		m_CommandList.Get(), vertices.data(), vbByteSize, m_BoxGeo->VertexBufferUploader);

	m_BoxGeo->IndexBufferGPU = CreateDefaultBuffer(m_Device.Get(),
		m_CommandList.Get(), indices.data(), ibByteSize, m_BoxGeo->IndexBufferUploader);

	m_BoxGeo->VertexByteStride = sizeof(Vertex);
	m_BoxGeo->VertexBufferByteSize = vbByteSize;
	m_BoxGeo->IndexFormat = DXGI_FORMAT_R16_UINT;
	m_BoxGeo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	m_BoxGeo->DrawArgs["box"] = submesh;
}
