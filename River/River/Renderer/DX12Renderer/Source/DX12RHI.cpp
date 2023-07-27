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
#include "DirectXCollision.h"
#include <d3dcompiler.h>
#include <iostream>
#include <chrono>

#define DEFAULT_SHADER_PATH "F:\\GitHub\\River\\River\\Shaders\\Default.hlsl"
#define DEFAULT_TEXTURE_PATH_1 "F:\\GitHub\\River\\River\\Textures\\bricks.dds"
#define DEFAULT_TEXTURE_PATH_2 "F:\\GitHub\\River\\River\\Textures\\stone.dds"
#define DEFAULT_TEXTURE_PATH_3 "F:\\GitHub\\River\\River\\Textures\\tile.dds"
#define DEFAULT_TEXTURE_PATH_4 "F:\\GitHub\\River\\River\\Textures\\grass.dds"
#define DEFAULT_TEXTURE_PATH_5 "F:\\GitHub\\River\\River\\Textures\\water1.dds"
#define DEFAULT_TEXTURE_PATH_6 "F:\\GitHub\\River\\River\\Textures\\WireFence.dds"
#define DEFAULT_TEXTURE_PATH_7 "F:\\GitHub\\River\\River\\Textures\\bricks3.dds"
#define DEFAULT_TEXTURE_PATH_8 "F:\\GitHub\\River\\River\\Textures\\checkboard.dds"
#define DEFAULT_TEXTURE_PATH_9 "F:\\GitHub\\River\\River\\Textures\\ice.dds"
#define DEFAULT_TEXTURE_PATH_10 "F:\\GitHub\\River\\River\\Textures\\white1x1.dds"
#define DEFAULT_TEXTURE_PATH_11 "F:\\GitHub\\River\\River\\Textures\\WoodCrate01.dds"
#define DEFAULT_MODEL_PATH_1 "F:\\GitHub\\River\\River\\Models\\skull.txt"


PassUniform mMainPassCB;
DirectX::BoundingFrustum mCamFrustum;

UINT mInstanceCount = 0;

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

	m_PrespectiveCamera.SetPosition(0.0f, 2.0f, -15.0f);

	InitializeBase(param);

	ThrowIfFailed(m_CommandList->Reset(m_CommandAllocator.Get(), nullptr));

	{
		DX12GeometryGenerator::Get()->Initialize();
		DX12GeometryGenerator::Get()->LoadMeshByFile(DEFAULT_MODEL_PATH_1, "Skull");

		LoadTextures();

		InitBaseRootSignatures();
		
		InitDescriptorHeaps();
		
		InitBaseShaders();
		
		InitBaseMaterials();
		
		InitBaseRenderItems();
		
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
	{
		DirectX::XMMATRIX view = m_PrespectiveCamera.GetView();
		auto determinantView = XMMatrixDeterminant(view);
		DirectX::XMMATRIX invView = XMMatrixInverse(&determinantView, view);

		auto currInstanceBuffer = m_CurrFrameResource->m_InstanceUniform.get();
		for (auto& e : m_RenderItems)
		{
			int visibleInstanceCount = 0;
			for (auto& it : e.second->Instances)
			{
				DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&it.World);
				DirectX::XMMATRIX texTransform = DirectX::XMLoadFloat4x4(&it.TexTransform);

				auto determinantWorld = DirectX::XMMatrixDeterminant(world);
				DirectX::XMMATRIX invWorld = DirectX::XMMatrixInverse(&determinantWorld, world);

				// View space to the object's local space.
				DirectX::XMMATRIX viewToLocal = XMMatrixMultiply(invView, invWorld);

				// Transform the camera frustum from view space to the object's local space.
				DirectX::BoundingFrustum localSpaceFrustum;
				mCamFrustum.Transform(localSpaceFrustum, viewToLocal);


				if ((localSpaceFrustum.Contains(e.second->Bounds) != DirectX::DISJOINT) || true)
				{
					InstanceUniform data;
					XMStoreFloat4x4(&data.World, XMMatrixTranspose(world));
					XMStoreFloat4x4(&data.TexTransform, XMMatrixTranspose(texTransform));
					data.MaterialIndex = it.MaterialIndex;

					// Write the instance data to structured buffer for the visible objects.
					currInstanceBuffer->CopyData(visibleInstanceCount++, data);
				}
			}

			e.second->InstanceCount = visibleInstanceCount;
		}
	}
	//UpdateObjectCBs();
	UpdateMaterialCBs();

	UpdateMainPass(time);
}

void DX12RHI::Render()
{
	auto cmdListAlloc = m_CurrFrameResource->m_CommandAlloc;

	ThrowIfFailed(cmdListAlloc->Reset());

	ThrowIfFailed(m_CommandList->Reset(cmdListAlloc.Get(), m_PSOs["opaque"]->GetPSO()));

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

	auto matBuffer = m_CurrFrameResource->m_MaterialUniform->Resource();
	m_CommandList->SetGraphicsRootShaderResourceView(1, matBuffer->GetGPUVirtualAddress());

	m_CommandList->SetGraphicsRootDescriptorTable(3, m_SrvHeap->GetGPUDescriptorHandleForHeapStart());

	DrawRenderItems(m_CommandList.Get(), { m_RenderItems["skull"].get()});

	/*m_CommandList->OMSetStencilRef(1);
	m_CommandList->SetPipelineState(m_PSOs["markStencilMirrors"]->GetPSO());
	DrawRenderItems(m_CommandList.Get(), m_RenderLayers[(int)RenderLayer::Mirrors]);

	UINT passCBByteSize = RendererUtil::CalcMinimumGPUAllocSize(sizeof(PassUniform));
	m_CommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress() + 1 * passCBByteSize);
	m_CommandList->SetPipelineState(m_PSOs["drawStencilReflections"]->GetPSO());
	DrawRenderItems(m_CommandList.Get(), m_RenderLayers[(int)RenderLayer::Reflected]);

	m_CommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
	m_CommandList->OMSetStencilRef(0);*/

	/*m_CommandList->SetPipelineState(m_PSOs["transparent"]->GetPSO());
	DrawRenderItems(m_CommandList.Get(), m_RenderLayers[(int)RenderLayer::Transparent]);*/

	/*m_CommandList->SetPipelineState(m_PSOs["shadow"]->GetPSO());
	DrawRenderItems(m_CommandList.Get(), m_RenderLayers[(int)RenderLayer::Shadow]);*/

	auto rb2 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_CommandList->ResourceBarrier(1, &rb2);

	ThrowIfFailed(m_CommandList->Close());

	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	ThrowIfFailed(m_SwapChain->Present(0, 0));
	m_CurrBackBuffer = (m_CurrBackBuffer + 1) % s_SwapChainBufferCount;

	m_CurrFrameResource->m_FenceValue = ++m_CurrentFence;

	m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence);
}

Unique<PipelineState> DX12RHI::BuildPSO(Share<Shader> vsShader, Share<Shader> psShader, const V_Array<ShaderLayout>& Layout)
{
	return nullptr;
}

Unique<VertexBuffer> DX12RHI::CreateVertexBuffer(float* vertices, uint32_t size, uint32_t elementSize, const VertexBufferLayout& layout)
{
	return MakeUnique<DX12VertexBuffer>(m_Device.Get(), m_CommandList.Get(), vertices, size, elementSize, layout);
}

Unique<IndexBuffer> DX12RHI::CreateIndexBuffer(uint32_t* indices, uint32_t count, ShaderDataType indiceDataType)
{
	return MakeUnique<DX12IndexBuffer>(m_Device.Get(), m_CommandList.Get(), indices, count, indiceDataType);
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
	m_Textures["stoneTex"] = MakeUnique<DX12Texture>(m_Device.Get(), m_CommandList.Get(), "checkboardTex", DEFAULT_TEXTURE_PATH_2);
	m_Textures["tileTex"] = MakeUnique<DX12Texture>(m_Device.Get(), m_CommandList.Get(), "iceTex", DEFAULT_TEXTURE_PATH_3);
	m_Textures["crateTex"] = MakeUnique<DX12Texture>(m_Device.Get(), m_CommandList.Get(), "white1x1Tex", DEFAULT_TEXTURE_PATH_11);
	m_Textures["iceTex"] = MakeUnique<DX12Texture>(m_Device.Get(), m_CommandList.Get(), "bricksTex", DEFAULT_TEXTURE_PATH_9);
	m_Textures["grassTex"] = MakeUnique<DX12Texture>(m_Device.Get(), m_CommandList.Get(), "checkboardTex", DEFAULT_TEXTURE_PATH_4);
	m_Textures["defaultTex"] = MakeUnique<DX12Texture>(m_Device.Get(), m_CommandList.Get(), "defaultTex", DEFAULT_TEXTURE_PATH_10);
}

void DX12RHI::InitBaseMaterials()
{
	m_BaseMaterials["bricks0"] = MakeUnique<Material>("bricks0");
	m_BaseMaterials["bricks0"]->InitBaseParam({ 1.0f, 1.0f, 1.0f, 1.0f }, { 0.02f, 0.02f, 0.02f }, 0.1f, 0, 0);

	m_BaseMaterials["stone0"] = MakeUnique<Material>("stone0");
	m_BaseMaterials["stone0"]->InitBaseParam({ 1.0f, 1.0f, 1.0f, 0.5f }, { 0.05f, 0.05f, 0.05f }, 0.3f, 1, 1);
	
	m_BaseMaterials["tile0"] = MakeUnique<Material>("tile0");
	m_BaseMaterials["tile0"]->InitBaseParam({ 1.0f, 1.0f, 1.0f, 1.0f }, { 0.02f, 0.02f, 0.02f }, 0.3f, 2, 2);
	
	m_BaseMaterials["crate0"] = MakeUnique<Material>("crate0");
	m_BaseMaterials["crate0"]->InitBaseParam({ 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.2f, 3, 3);

	m_BaseMaterials["ice0"] = MakeUnique<Material>("ice0");
	m_BaseMaterials["ice0"]->InitBaseParam({ 1.0f, 1.0f, 1.0f, 1.0f }, { 0.1f, 0.1f, 0.1f }, 0.0f, 4, 4);

	m_BaseMaterials["grass0"] = MakeUnique<Material>("grass0");
	m_BaseMaterials["grass0"]->InitBaseParam({ 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.2f, 5, 5);

	m_BaseMaterials["skullMat"] = MakeUnique<Material>("skullMat");
	m_BaseMaterials["skullMat"]->InitBaseParam({ 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.5f, 6, 6);
}

void DX12RHI::InitBaseShaders()
{
	const D3D_SHADER_MACRO defines[] =
	{
		"ALPHA_TEST", "1",
		NULL, NULL
	};
	m_Shaders["standardVS"] = MakeShare<DX12Shader>(DEFAULT_SHADER_PATH, nullptr, "VS", "vs_5_1");
	m_Shaders["opaquePS"] = MakeShare<DX12Shader>(DEFAULT_SHADER_PATH, defines, "PS", "ps_5_1");

	/*const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"FOG", "1",
		"ALPHA_TEST", "1",
		NULL, NULL
	};
	m_Shaders["alphaTestedPS"] = MakeShare<DX12Shader>(DEFAULT_SHADER_PATH, alphaTestDefines, "PS", "ps_5_0");*/
}

void DX12RHI::InitBaseRootSignatures()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, (UINT)m_Textures.size(), 0, 0);

	CD3DX12_ROOT_PARAMETER slotRootParameter[4];

	slotRootParameter[0].InitAsShaderResourceView(0, 1);
	slotRootParameter[1].InitAsShaderResourceView(1, 1);
	slotRootParameter[2].InitAsConstantBufferView(0);
	slotRootParameter[3].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);

	auto staticSamplers = DX12RootSignature::GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter, (UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	m_RootSignatures["default"] = MakeShare<DX12RootSignature>(m_Device.Get(), rootSigDesc);
}

void DX12RHI::InitRenderLayers()
{
	/*for (auto& it : m_RenderItems)
	{
		for (auto& instance : it.second->Instances)
		{
			m_RenderLayers[(int)RenderLayer::Opaque].push_back(&instance.second);
		}
	}*/
}

void DX12RHI::InitBasePSOs()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	{
		ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		opaquePsoDesc.InputLayout = { inputLayout.data(), (uint32_t)inputLayout.size() };
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
	
	//D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentPsoDesc = opaquePsoDesc;
	//{

	//	D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
	//	transparencyBlendDesc.BlendEnable = true;
	//	transparencyBlendDesc.LogicOpEnable = false;
	//	transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	//	transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	//	transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	//	transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	//	transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	//	transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	//	transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	//	transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	//	transparentPsoDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
	//	
	//	m_PSOs["transparent"] = MakeShare<DX12PipelineState>(m_Device.Get(), transparentPsoDesc);
	//}

	//{
	//	D3D12_GRAPHICS_PIPELINE_STATE_DESC markMirrorsPsoDesc = opaquePsoDesc;
	//	
	//	CD3DX12_BLEND_DESC mirrorBlendState(D3D12_DEFAULT);
	//	mirrorBlendState.RenderTarget[0].RenderTargetWriteMask = 0;

	//	D3D12_DEPTH_STENCIL_DESC mirrorDSS;
	//	mirrorDSS.DepthEnable = true;
	//	mirrorDSS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	//	mirrorDSS.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	//	mirrorDSS.StencilEnable = true;
	//	mirrorDSS.StencilReadMask = 0xff;
	//	mirrorDSS.StencilWriteMask = 0xff;

	//	mirrorDSS.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	//	mirrorDSS.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	//	mirrorDSS.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	//	mirrorDSS.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	//	// We are not rendering backfacing polygons, so these settings do not matter.
	//	mirrorDSS.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	//	mirrorDSS.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	//	mirrorDSS.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	//	mirrorDSS.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	//	markMirrorsPsoDesc.BlendState = mirrorBlendState;
	//	markMirrorsPsoDesc.DepthStencilState = mirrorDSS;

	//	m_PSOs["markStencilMirrors"] = MakeShare<DX12PipelineState>(m_Device.Get(), markMirrorsPsoDesc);
	//}
	//
	//{
	//	D3D12_GRAPHICS_PIPELINE_STATE_DESC drawReflectionsPsoDesc = opaquePsoDesc;

	//	D3D12_DEPTH_STENCIL_DESC reflectionsDSS;
	//	reflectionsDSS.DepthEnable = true;
	//	reflectionsDSS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//	reflectionsDSS.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	//	reflectionsDSS.StencilEnable = true;
	//	reflectionsDSS.StencilReadMask = 0xff;
	//	reflectionsDSS.StencilWriteMask = 0xff;

	//	reflectionsDSS.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	//	reflectionsDSS.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	//	reflectionsDSS.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	//	reflectionsDSS.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	//	// We are not rendering backfacing polygons, so these settings do not matter.
	//	reflectionsDSS.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	//	reflectionsDSS.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	//	reflectionsDSS.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	//	reflectionsDSS.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	//	drawReflectionsPsoDesc.DepthStencilState = reflectionsDSS;
	//	drawReflectionsPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	//	drawReflectionsPsoDesc.RasterizerState.FrontCounterClockwise = true;

	//	m_PSOs["drawStencilReflections"] = MakeShare<DX12PipelineState>(m_Device.Get(), drawReflectionsPsoDesc);
	//}

	//{
	//	D3D12_GRAPHICS_PIPELINE_STATE_DESC shadowPsoDesc = transparentPsoDesc;

	//	D3D12_DEPTH_STENCIL_DESC shadowDSS;
	//	shadowDSS.DepthEnable = true;
	//	shadowDSS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//	shadowDSS.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	//	shadowDSS.StencilEnable = true;
	//	shadowDSS.StencilReadMask = 0xff;
	//	shadowDSS.StencilWriteMask = 0xff;

	//	shadowDSS.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	//	shadowDSS.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	//	shadowDSS.FrontFace.StencilPassOp = D3D12_STENCIL_OP_INCR;
	//	shadowDSS.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	//	// We are not rendering backfacing polygons, so these settings do not matter.
	//	shadowDSS.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	//	shadowDSS.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	//	shadowDSS.BackFace.StencilPassOp = D3D12_STENCIL_OP_INCR;
	//	shadowDSS.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	//	shadowPsoDesc.DepthStencilState = shadowDSS;

	//	m_PSOs["shadow"] = MakeShare<DX12PipelineState>(m_Device.Get(), shadowPsoDesc);
	//}
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
	/*auto waterMat = m_BaseMaterials["water"].get();

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

	waterMat->NumFramesDirty = 3;*/
}

void DX12RHI::UpdateObjectCBs()
{
	/*auto& currObjectCB = m_CurrFrameResource->m_ObjectUniform;
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 texMatrix;

	for (auto& it : m_RenderLayers)
	{
		for (size_t i = 0; i < it.size(); i++)
		{
			if (it[i]->NumFramesDirty > 0)
			{
				memcpy(&worldMatrix, &it[i]->m_WorldTransform, sizeof(worldMatrix));
				memcpy(&texMatrix, &it[i]->m_TexTransform, sizeof(texMatrix));
				DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&worldMatrix);
				DirectX::XMMATRIX texTransform = DirectX::XMLoadFloat4x4(&texMatrix);

				ObjectUniform  objConstants;
				XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(world));
				XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));

				currObjectCB->CopyData(it[i]->m_ObjectIndex, objConstants);

				it[i]->NumFramesDirty--;
			}
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
			matConstants.DiffuseMapIndex = mat->DiffuseSrvHeapIndex;
			XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

			currMaterialCB->CopyData(mat->MatCBIndex, matConstants);

			mat->NumFramesDirty--;
		}
	}
}

void DX12RHI::UpdateMainPass(const RiverTime& time)
{
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

	// Add an instruction to the command queue to set a new fence point.  Because we
	// are on the GPU timeline, the new fence point won't be set until the GPU finishes
	// processing all the commands prior to this Signal().
	ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence));

	// Wait until the GPU has completed commands up to this fence point.
	if (m_Fence->GetCompletedValue() < m_CurrentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

		// Fire event when GPU hits current fence.
		ThrowIfFailed(m_Fence->SetEventOnCompletion(m_CurrentFence, eventHandle));

		// Wait until the GPU hits current fence event is fired.
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

void DX12RHI::InitDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = (int)m_Textures.size();
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(m_Device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_SrvHeap)));

	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(m_SrvHeap->GetCPUDescriptorHandleForHeapStart());

	auto bricksTex = m_Textures["bricksTex"]->GetResource();
	auto stoneTex = m_Textures["stoneTex"]->GetResource();
	auto tileTex = m_Textures["tileTex"]->GetResource();
	auto crateTex = m_Textures["crateTex"]->GetResource();
	auto iceTex = m_Textures["iceTex"]->GetResource();
	auto grassTex = m_Textures["grassTex"]->GetResource();
	auto defaultTex = m_Textures["defaultTex"]->GetResource();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = bricksTex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = bricksTex->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	m_Device->CreateShaderResourceView(bricksTex.Get(), &srvDesc, hDescriptor);

	hDescriptor.Offset(1, m_CbvSrvUavDescriptorSize);

	srvDesc.Format = stoneTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = stoneTex->GetDesc().MipLevels;
	m_Device->CreateShaderResourceView(stoneTex.Get(), &srvDesc, hDescriptor);

	hDescriptor.Offset(1, m_CbvSrvUavDescriptorSize);

	srvDesc.Format = tileTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = tileTex->GetDesc().MipLevels;
	m_Device->CreateShaderResourceView(tileTex.Get(), &srvDesc, hDescriptor);

	hDescriptor.Offset(1, m_CbvSrvUavDescriptorSize);

	srvDesc.Format = crateTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = crateTex->GetDesc().MipLevels;
	m_Device->CreateShaderResourceView(crateTex.Get(), &srvDesc, hDescriptor);

	hDescriptor.Offset(1, m_CbvSrvUavDescriptorSize);

	srvDesc.Format = iceTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = iceTex->GetDesc().MipLevels;
	m_Device->CreateShaderResourceView(iceTex.Get(), &srvDesc, hDescriptor);

	hDescriptor.Offset(1, m_CbvSrvUavDescriptorSize);

	srvDesc.Format = grassTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = grassTex->GetDesc().MipLevels;
	m_Device->CreateShaderResourceView(grassTex.Get(), &srvDesc, hDescriptor);

	hDescriptor.Offset(1, m_CbvSrvUavDescriptorSize);

	srvDesc.Format = defaultTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = defaultTex->GetDesc().MipLevels;
	m_Device->CreateShaderResourceView(defaultTex.Get(), &srvDesc, hDescriptor);
}

void DX12RHI::InitBaseRenderItems()
{
	{
		auto& geo = DX12GeometryGenerator::Get()->GetMesh("Skull");
		auto skullRenderItem = MakeUnique<DX12RenderItem>();
		skullRenderItem->World = Identity4x4();
		skullRenderItem->TexTransform = Identity4x4();
		skullRenderItem->ObjCBIndex = 0;
		skullRenderItem->Mat = m_BaseMaterials["tile0"].get();
		skullRenderItem->Geo = geo.get();
		skullRenderItem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		skullRenderItem->InstanceCount = 0;
		skullRenderItem->IndexCount = geo->DrawArgs["skull"].IndexCount;
		skullRenderItem->StartIndexLocation = geo->DrawArgs["skull"].StartIndexLocation;
		skullRenderItem->BaseVertexLocation = geo->DrawArgs["skull"].BaseVertexLocation;
		skullRenderItem->Bounds = geo->DrawArgs["skull"].Bounds;

		const int n = 2;
		mInstanceCount = n * n * n;
		skullRenderItem->Instances.resize(mInstanceCount);

		float width = 200.0f;
		float height = 200.0f;
		float depth = 200.0f;

		float x = -0.5f * width;
		float y = -0.5f * height;
		float z = -0.5f * depth;
		float dx = width / (n - 1);
		float dy = height / (n - 1);
		float dz = depth / (n - 1);
		for (int k = 0; k < n; ++k)
		{
			for (int i = 0; i < n; ++i)
			{
				for (int j = 0; j < n; ++j)
				{
					int index = k * n * n + i * n + j;

					skullRenderItem->Instances[index].World = DirectX::XMFLOAT4X4(
						1.0f, 0.0f, 0.0f, 0.0f,
						0.0f, 1.0f, 0.0f, 0.0f,
						0.0f, 0.0f, 1.0f, 0.0f,
						x + j * dx, y + i * dy, z + k * dz, 1.0f);

					DirectX::XMStoreFloat4x4(&skullRenderItem->Instances[index].TexTransform, DirectX::XMMatrixScaling(2.0f, 2.0f, 1.0f));
					skullRenderItem->Instances[index].MaterialIndex = index % m_BaseMaterials.size();
				}
			}
		}

		m_RenderItems["skull"] = std::move(skullRenderItem);
		m_RenderLayers[(int)RenderLayer::Opaque].push_back(m_RenderItems["skull"].get());
	}
}

void DX12RHI::InitFrameBuffer()
{
	for (int i = 0; i < s_FrameBufferCount; ++i)
	{
		m_FrameBuffer.push_back(MakeUnique<DX12FrameBuffer>(m_Device.Get(), 1, s_MaxRenderItem, 
			(int)m_BaseMaterials.size()));
	}
}

void DX12RHI::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const V_Array<DX12RenderItem*>& items)
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

		vertexBuffer = dynamic_cast<DX12VertexBuffer*>(ri->Geo->VertexBuffer.get());
		indexBuffer = dynamic_cast<DX12IndexBuffer*>(ri->Geo->IndexBufer.get());

		cmdList->IASetVertexBuffers(0, 1, &vertexBuffer->m_VertexBufferView);
		cmdList->IASetIndexBuffer(&indexBuffer->m_IndexBufferView);
		cmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Set the instance buffer to use for this render-item.  For structured buffers, we can bypass 
		// the heap and set as a root descriptor.
		auto instanceBuffer = m_CurrFrameResource->m_InstanceUniform->Resource();
		cmdList->SetGraphicsRootShaderResourceView(0, instanceBuffer->GetGPUVirtualAddress());

		cmdList->DrawIndexedInstanced(ri->IndexCount, ri->InstanceCount, 0, 0, 0);
	}
}

//void DX12RHI::DrawRenderItemInstances(ID3D12GraphicsCommandList* cmdList, const V_Array<RenderItemInstance*>& instances)
//{
//	UINT objCBByteSize = RendererUtil::CalcMinimumGPUAllocSize(sizeof(ObjectUniform));
//	UINT matCBByteSize = RendererUtil::CalcMinimumGPUAllocSize(sizeof(MaterialUniform));
//
//	auto objectCB = m_CurrFrameResource->m_ObjectUniform->Resource();
//	auto matCB = m_CurrFrameResource->m_MaterialUniform->Resource();
//
//	// For each render item...
//	DX12VertexBuffer* vertexBuffer = nullptr;
//	DX12IndexBuffer* indexBuffer = nullptr;
//	for (size_t i = 0; i < instances.size(); ++i)
//	{
//		auto ri = instances[i];
//
//		vertexBuffer = dynamic_cast<DX12VertexBuffer*>(ri->m_RenderItem->m_VertexBuffer);
//		indexBuffer = dynamic_cast<DX12IndexBuffer*>(ri->m_RenderItem->m_IndexBuffer);
//
//		cmdList->IASetVertexBuffers(0, 1, &vertexBuffer->m_VertexBufferView);
//
//		cmdList->IASetIndexBuffer(&indexBuffer->m_IndexBufferView);
//		cmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//		/*CD3DX12_GPU_DESCRIPTOR_HANDLE tex(m_SrvHeap->GetGPUDescriptorHandleForHeapStart());
//		tex.Offset(ri->m_Material->DiffuseSrvHeapIndex, m_CbvSrvUavDescriptorSize);*/
//
//		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->m_ObjectIndex * objCBByteSize;
//		//D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->m_Material->MatCBIndex * matCBByteSize;
//
//		cmdList->SetGraphicsRootShaderResourceView(0, objCBAddress);
//
//		cmdList->DrawIndexedInstanced(ri->IndexCount, ri->InstanceCount, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
//	}
//}
