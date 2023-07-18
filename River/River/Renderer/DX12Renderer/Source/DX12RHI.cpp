#include "RiverPch.h"
#include "RendererUtil.h"
#include "Renderer/DX12Renderer/Header/DX12Util.h"
#include "Renderer/DX12Renderer/Header/d3dx12.h"
#include "Renderer/DX12Renderer/Header/DX12RHI.h"
#include "Renderer/DX12Renderer/Header/DX12PipelineState.h"
#include "Renderer/DX12Renderer/Header/DX12UniformBuffer.h"
#include "Renderer/DX12Renderer/Header/DX12MeshGeometry.h"
#include "Renderer/DX12Renderer/Header/DX12VertexBuffer.h"
#include "DirectXMath.h"
#include "Renderer/DX12Renderer/Header/DX12Shader.h"

#include <d3dcompiler.h>

#include "strsafe.h"

struct Vertex
{
	DirectX::XMFLOAT4 Pos;
	DirectX::XMFLOAT4 Color;
};

struct ObjectConstants
{
	DirectX::XMFLOAT4X4 WorldViewProj = DirectX::XMFLOAT4X4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
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
float fTrangleSize = 3.0f;

DX12RHI::DX12RHI() : m_CurrentFence(0), m_CurrBackBuffer(0), m_RtvDescriptorSize(0), m_DsvDescriptorSize(0), m_CbvSrvUavDescriptorSize(0)
{
}

DX12RHI::~DX12RHI()
{
}

Share<DX12VertexBuffer> mVertexBuffer;

void DX12RHI::Initialize(const RHIInitializeParam& param)
{
	m_Viewport = { 0.0f, 0.0f, static_cast<float>(param.WindowWidth), static_cast<float>(param.WindowHeight), D3D12_MIN_DEPTH, D3D12_MAX_DEPTH };
	m_ScissorRect = { 0, 0, static_cast<LONG>(param.WindowWidth), static_cast<LONG>(param.WindowHeight) };

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_Factory)));

	EnumAdaptersAndCreateDevice();

	CreateCommandQueue();

	CreateSwapChain(param);

	CreateRtvHeapAndDescriptor();

	/*m_DsvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_CbvSrvUavDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);*/

	auto Shader = MakeShare<DX12Shader>("F:\\GitHub\\River\\River\\Shaders\\shaders.hlsl");
	m_PSOs.push_back(MakeShare<DX12PipelineState>(m_Device.Get(), Shader));

	ThrowIfFailed(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator)));
	ThrowIfFailed(m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(),
		m_PSOs[0]->m_PipelineState.Get(), IID_PPV_ARGS(&m_CommandList)));

	// 定义三角形的3D数据结构，每个顶点使用三原色之一
	GRS_VERTEX stTriangleVertices[] =
	{
		{ { 0.0f, 0.25f * fTrangleSize, 0.0f ,1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		{ { 0.25f * fTrangleSize, -0.25f * fTrangleSize, 0.0f ,1.0f  }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		{ { -0.25f * fTrangleSize, -0.25f * fTrangleSize, 0.0f  ,1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
	};

	mVertexBuffer = MakeShare<DX12VertexBuffer>(m_Device.Get(), (float*)(&stTriangleVertices),
		(uint32_t)sizeof(stTriangleVertices), (uint32_t)sizeof(GRS_VERTEX));
	//auto mDX12VertexBuffer = dynamic_cast<DX12VertexBuffer*>(VertexBuffer.get());

	CreateFence();
	DWORD dwRet = 0;
	ThrowIfFailed(m_CommandList->Close());
	SetEvent(hEventFence);
}

void DX12RHI::OnUpdate()
{
	MSG	msg = {};
	D3D12_CPU_DESCRIPTOR_HANDLE stRTVHandle = m_RtvHeap->GetCPUDescriptorHandleForHeapStart();
	// 填充资源屏障结构
	D3D12_RESOURCE_BARRIER stBeginResBarrier = {};
	stBeginResBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	stBeginResBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	stBeginResBarrier.Transition.pResource = m_SwapChainBuffer[m_CurrBackBuffer].Get();
	stBeginResBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	stBeginResBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	stBeginResBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	D3D12_RESOURCE_BARRIER stEndResBarrier = {};
	stEndResBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	stEndResBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	stEndResBarrier.Transition.pResource = m_SwapChainBuffer[m_CurrBackBuffer].Get();
	stEndResBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	stEndResBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	stEndResBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	DWORD dwRet = ::MsgWaitForMultipleObjects(1, &hEventFence, FALSE, INFINITE, QS_ALLINPUT);

	if (dwRet - WAIT_OBJECT_0 == 0)
	{
		//获取新的后缓冲序号，因为Present真正完成时后缓冲的序号就更新了
		m_CurrBackBuffer = m_SwapChain3->GetCurrentBackBufferIndex();

		//命令分配器先Reset一下
		ThrowIfFailed(m_CommandAllocator->Reset());
		//Reset命令列表，并重新指定命令分配器和PSO对象
		ThrowIfFailed(m_CommandList->Reset(m_CommandAllocator.Get(), m_PSOs[0]->m_PipelineState.Get()));

		//开始记录命令
		m_CommandList->SetGraphicsRootSignature(m_PSOs[0]->m_RootSignature.Get());
		m_CommandList->SetPipelineState(m_PSOs[0]->m_PipelineState.Get());
		m_CommandList->RSSetViewports(1, &m_Viewport);
		m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

		// 通过资源屏障判定后缓冲已经切换完毕可以开始渲染了
		stBeginResBarrier.Transition.pResource = m_SwapChainBuffer[m_CurrBackBuffer].Get();
		m_CommandList->ResourceBarrier(1, &stBeginResBarrier);

		stRTVHandle = m_RtvHeap->GetCPUDescriptorHandleForHeapStart();
		stRTVHandle.ptr += m_CurrBackBuffer * m_RtvDescriptorSize;
		//设置渲染目标
		m_CommandList->OMSetRenderTargets(1, &stRTVHandle, FALSE, nullptr);

		// 继续记录命令，并真正开始新一帧的渲染
		const float	faClearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		m_CommandList->ClearRenderTargetView(stRTVHandle, faClearColor, 0, nullptr);
		m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		m_CommandList->IASetVertexBuffers(0, 1, &mVertexBuffer->mVertexBufferView);

		//Draw Call！！！
		m_CommandList->DrawInstanced(3, 1, 0, 0);

		//又一个资源屏障，用于确定渲染已经结束可以提交画面去显示了
		stEndResBarrier.Transition.pResource = m_SwapChainBuffer[m_CurrBackBuffer].Get();
		m_CommandList->ResourceBarrier(1, &stEndResBarrier);
		//关闭命令列表，可以去执行了
		ThrowIfFailed(m_CommandList->Close());

		//执行命令列表
		ID3D12CommandList* ppCommandLists[] = { m_CommandList.Get() };
		m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		//提交画面
		ThrowIfFailed(m_SwapChain3->Present(1, 0));

		//开始同步GPU与CPU的执行，先记录围栏标记值
		const UINT64 n64CurrentFenceValue = m_CurrentFence;
		ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), n64CurrentFenceValue));
		m_CurrentFence++;
		ThrowIfFailed(m_Fence->SetEventOnCompletion(n64CurrentFenceValue, hEventFence));
	}
	/*else if (dwRet - WAIT_OBJECT_0 == 1)
	{
		while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (WM_QUIT != msg.message)
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}
	}*/
}

void DX12RHI::Render()
{
	float mRadius = 5.0f;
	float mTheta = 1.5f * 3.141592654f;
	float mPhi = 0.785398163f;
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	// Build the view matrix.
	DirectX::XMVECTOR pos = DirectX::XMVectorSet(x, y, z, 1.0f);
	DirectX::XMVECTOR target = DirectX::XMVectorZero();
	DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, view);

	DirectX::XMMATRIX world = XMLoadFloat4x4(&mWorld);
	DirectX::XMMATRIX proj = XMLoadFloat4x4(&mProj);
	DirectX::XMMATRIX worldViewProj = world * view * proj;

	// Update the constant buffer with the latest worldViewProj matrix.
	ObjectConstants objConstants;
	XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(worldViewProj));
	m_UniformBuffer->CopyData(0, &objConstants, sizeof(objConstants));

	for (auto& pso : m_PSOs)
	{
		m_CommandAllocator->Reset();

		m_CommandList->Reset(m_CommandAllocator.Get(), pso->m_PipelineState.Get());

		m_CommandList->RSSetViewports(1, &m_Viewport);
		m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

		// Indicate a state transition on the resource usage.
		auto rb_p2t = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		auto rb_t2p = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

		m_CommandList->ResourceBarrier(1, &rb_p2t);

		// Clear the back buffer and depth buffer.
		auto DsvHeapStart = m_DsvHeap->GetCPUDescriptorHandleForHeapStart();
		auto backBufferView = CurrentBackBufferView();

		m_CommandList->ClearRenderTargetView(backBufferView, DirectX::Colors::LightYellow, 0, nullptr);
		m_CommandList->ClearDepthStencilView(DsvHeapStart,
			D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		// Specify the buffers we are going to render to.
		m_CommandList->OMSetRenderTargets(1, &backBufferView, true, &DsvHeapStart);

		ID3D12DescriptorHeap* descriptorHeaps[] = { m_CbvHeap.Get() };
		m_CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

		m_CommandList->SetGraphicsRootSignature(pso->m_RootSignature.Get());

		auto vbv = m_BoxGeo->VertexBufferView();
		auto ibv = m_BoxGeo->IndexBufferView();
		m_CommandList->IASetVertexBuffers(0, 1, &vbv);
		m_CommandList->IASetIndexBuffer(&ibv);
		m_CommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		m_CommandList->SetGraphicsRootDescriptorTable(0, m_CbvHeap->GetGPUDescriptorHandleForHeapStart());

		m_CommandList->DrawIndexedInstanced(m_BoxGeo->DrawArgs["box"].IndexCount,
			1, 0, 0, 0);

		m_CommandList->ResourceBarrier(1, &rb_t2p);

		m_CommandList->Close();

		ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
		m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		//m_SwapChain->Present(0, 0);
		m_CurrBackBuffer = (m_CurrBackBuffer + 1) % s_SwapChainBufferCount;

		FlushCommandQueue();
	}
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
	m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);

	for (size_t i = 0; i < s_SwapChainBufferCount; i++)
	{
		m_SwapChainBuffer[i].Reset();
	}
	m_DepthStencilBuffer.Reset();

	//m_SwapChain->ResizeBuffers(s_SwapChainBufferCount, param.WindowWidth, param.WindowHeight, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
	m_CurrBackBuffer = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < s_SwapChainBufferCount; i++)
	{
		//m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_SwapChainBuffer[i]));
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
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
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

	m_CommandList->Close();
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

	DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(0.25f * 3.1415926535f, static_cast<float>(param.WindowWidth) / param.WindowHeight, 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
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
	ThrowIfFailed(m_Device->CreateCommandQueue(&stQueueDesc, IID_PPV_ARGS(&m_CommandQueue)));
}

void DX12RHI::CreateSwapChain(const RHIInitializeParam& param)
{
	DXGI_SWAP_CHAIN_DESC1 stSwapChainDesc = {};
	stSwapChainDesc.BufferCount = s_SwapChainBufferCount;
	stSwapChainDesc.Width = param.WindowWidth;
	stSwapChainDesc.Height = param.WindowHeight;
	stSwapChainDesc.Format = m_RenderTargetFormat;
	stSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	stSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	stSwapChainDesc.SampleDesc.Count = 1;

	ThrowIfFailed(m_Factory->CreateSwapChainForHwnd(
		m_CommandQueue.Get(),		// 交换链需要命令队列，Present命令要执行
		(HWND)param.HWnd,
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

void DX12RHI::CreateRtvHeapAndDescriptor()
{
	D3D12_DESCRIPTOR_HEAP_DESC stRTVHeapDesc = {};
	stRTVHeapDesc.NumDescriptors = s_SwapChainBufferCount;
	stRTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	stRTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	ThrowIfFailed(m_Device->CreateDescriptorHeap(&stRTVHeapDesc, IID_PPV_ARGS(&m_RtvHeap)));

	//得到每个描述符元素的大小
	m_RtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	D3D12_CPU_DESCRIPTOR_HANDLE stRTVHandle = m_RtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < s_SwapChainBufferCount; i++)
	{
		ThrowIfFailed(m_SwapChain3->GetBuffer(i, IID_PPV_ARGS(&m_SwapChainBuffer[i])));

		m_Device->CreateRenderTargetView(m_SwapChainBuffer[i].Get(), nullptr, stRTVHandle);

		stRTVHandle.ptr += m_RtvDescriptorSize;
	}
}

void DX12RHI::CreateFence()
{
	ThrowIfFailed(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));
	m_CurrentFence = 1;

	// 创建一个Event同步对象，用于等待围栏事件通知
	hEventFence = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (hEventFence == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}
}

void DX12RHI::FlushCommandQueue()
{
	m_CurrentFence++;

	m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence);

	if (m_Fence->GetCompletedValue() < m_CurrentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

		m_Fence->SetEventOnCompletion(m_CurrentFence, eventHandle);

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
