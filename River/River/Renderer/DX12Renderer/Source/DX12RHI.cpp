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


DX12RHI::DX12RHI() : m_CurrentFence(0), m_CurrBackBuffer(0), m_RtvDescriptorSize(0), m_DsvDescriptorSize(0), m_CbvSrvUavDescriptorSize(0)
{
}

DX12RHI::~DX12RHI()
{
}

void DX12RHI::Initialize(const RHIInitializeParam& param)
{
	CreateDXGIFactory1(IID_PPV_ARGS(&m_Factory));

	HRESULT hardwareResult = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_Device));
	if (FAILED(hardwareResult))
	{
		Microsoft::WRL::ComPtr<IDXGIAdapter> pWarpAdapter;
		m_Factory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter));
		D3D12CreateDevice(pWarpAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_Device));
	}

	m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));

	m_RtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_DsvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_CbvSrvUavDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	m_Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msQualityLevels, sizeof(msQualityLevels));

	m_4xMsaaQuality = msQualityLevels.NumQualityLevels;
	assert(m_4xMsaaQuality > 0 && "Unexpected MSAA quality level.");


	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue));
	m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_CommandAllocator.GetAddressOf()));
	m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(m_CommandList.GetAddressOf()));
	m_CommandList->Close();

	CreateSwapChain(param);

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = s_SwapChainBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	m_Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(m_RtvHeap.GetAddressOf()));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	m_Device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(m_DsvHeap.GetAddressOf()));

	Resize(param);
	
	m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);

	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	m_Device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_CbvHeap));

	m_UniformBuffer = MakeUnique<DX12UniformBuffer>(m_Device.Get(), (unsigned int)sizeof(ObjectConstants),1, true);
	UINT objCBByteSize = RendererUtil::CalcMinimumGPUAllocSize(sizeof(ObjectConstants));

	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = m_UniformBuffer->Resource()->GetGPUVirtualAddress();
	int boxCBufIndex = 0;
	cbAddress += boxCBufIndex * objCBByteSize;

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = RendererUtil::CalcMinimumGPUAllocSize(sizeof(ObjectConstants));
	m_Device->CreateConstantBufferView(&cbvDesc, m_CbvHeap->GetCPUDescriptorHandleForHeapStart());

	BuildMeshGeometry();

	auto Shader = MakeShare<DX12Shader>("F:\\GitHub\\River\\River\\Shaders\\shaders.hlsl");

	m_PSOs.push_back(MakeShare<DX12PipelineState>(m_Device.Get(), Shader));

	m_CommandList->Close();
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	FlushCommandQueue();
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

		m_SwapChain->Present(0, 0);
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
	return MakeShare<DX12VertexBuffer>(m_Device.Get(), vertices, size);
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

	m_SwapChain->ResizeBuffers(s_SwapChainBufferCount, param.WindowWidth, param.WindowHeight, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
	m_CurrBackBuffer = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < s_SwapChainBufferCount; i++)
	{
		m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_SwapChainBuffer[i]));
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

void DX12RHI::CreateSwapChain(const RHIInitializeParam& param)
{
	m_SwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = param.WindowWidth;
	sd.BufferDesc.Height = param.WindowHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = s_SwapChainBufferCount;
	sd.OutputWindow = (HWND)param.HWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	m_Factory->CreateSwapChain(m_CommandQueue.Get(), &sd, m_SwapChain.GetAddressOf());
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

void DX12RHI::BuildMeshGeometry()
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
	m_BoxGeo->Name = "boxGeo";

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
