#include "RiverPch.h"
#include "RendererUtil.h"
#include "Renderer/DX12Renderer/Header/d3dx12.h"
#include "Renderer/DX12Renderer/Header/DX12RHI.h"
#include "Renderer/DX12Renderer/Header/DX12RootSignature.h"
#include "Renderer/DX12Renderer/Header/DX12PipelineState.h"
#include "Renderer/DX12Renderer/Header/DX12VertexBuffer.h"
#include "Renderer/DX12Renderer/Header/DX12IndexBuffer.h"
#include "Renderer/DX12Renderer/Header/DX12Shader.h"
#include "Renderer/DX12Renderer/Header/DX12GeometryGenerator.h"
#include "Renderer/DX12Renderer/Header/DX12Texture.h"
#include "Renderer/DX12Renderer/Header/DX12ShadowMap.h"
#include "Renderer/DX12Renderer/Header/DX12Ssao.h"
#include "Renderer/DX12Renderer/Header/Waves.h"
#include "Renderer/DX12Renderer/Header/DX12LoadM3d.h"
#include "Renderer/DX12Renderer/Header/DDSTextureLoader.h"

#include "DirectXMath.h"
#include "DirectXCollision.h"
#include <d3dcompiler.h>
#include <iostream>
#include <chrono>
#include <fstream>

using Microsoft::WRL::ComPtr;
using namespace DirectX;
#define DEFAULT_SHADER_PATH_1 "F:\\GitHub\\River\\River\\Shaders\\Default.hlsl"
#define DEFAULT_SHADER_PATH_2 "F:\\GitHub\\River\\River\\Shaders\\Sky.hlsl"
#define DEFAULT_SHADER_PATH_3 "F:\\GitHub\\River\\River\\Shaders\\Shadows.hlsl"
#define DEFAULT_SHADER_PATH_4 "F:\\GitHub\\River\\River\\Shaders\\ShadowDebug.hlsl"
#define DEFAULT_SHADER_PATH_5 "F:\\GitHub\\River\\River\\Shaders\\Ssao.hlsl"
#define DEFAULT_SHADER_PATH_6 "F:\\GitHub\\River\\River\\Shaders\\DrawNormals.hlsl"
#define DEFAULT_SHADER_PATH_7 "F:\\GitHub\\River\\River\\Shaders\\SsaoBlur.hlsl"

#define DEFAULT_TEXTURE_PATH   "F:\\GitHub\\River\\River\\Textures\\"
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
#define DEFAULT_TEXTURE_PATH_12 "F:\\GitHub\\River\\River\\Textures\\grasscube1024.dds"
#define DEFAULT_TEXTURE_PATH_13 "F:\\GitHub\\River\\River\\Textures\\bricks2.dds"
#define DEFAULT_TEXTURE_PATH_14 "F:\\GitHub\\River\\River\\Textures\\bricks2_nmap.dds"
#define DEFAULT_TEXTURE_PATH_15 "F:\\GitHub\\River\\River\\Textures\\tile_nmap.dds"
#define DEFAULT_TEXTURE_PATH_16 "F:\\GitHub\\River\\River\\Textures\\default_nmap.dds"
#define DEFAULT_TEXTURE_PATH_17 "F:\\GitHub\\River\\River\\Textures\\snowcube1024.dds"
#define DEFAULT_TEXTURE_PATH_18 "F:\\GitHub\\River\\River\\Textures\\desertcube1024.dds"

#define DEFAULT_MODEL_PATH_1 "F:\\GitHub\\River\\River\\Models\\skull.txt"
#define DEFAULT_MODEL_PATH_2 "F:\\GitHub\\River\\River\\Models\\car.txt"
#define DEFAULT_MODEL_PATH_3 "F:\\GitHub\\River\\River\\Models\\soldier.m3d"

DirectX::BoundingSphere mSceneBounds;
PassUniform mMainPassCB;
PassUniform mShadowPassCB;
float mLightNearZ = 0.0f;
float mLightFarZ = 0.0f;
DirectX::XMFLOAT3 mLightPosW;
DirectX::XMFLOAT4X4 mLightView = Identity4x4();
DirectX::XMFLOAT4X4 mLightProj = Identity4x4();
DirectX::XMFLOAT4X4 mShadowTransform = Identity4x4();

DirectX::BoundingFrustum mCamFrustum;
DX12RenderItem* mPickedRitem = nullptr;
UINT mSkyTexHeapIndex;
UINT mShadowMapHeapIndex;
UINT mSsaoHeapIndexStart = 0;
UINT mSsaoAmbientMapIndex = 0;
UINT mNullCubeSrvIndex = 0;
UINT mNullTexSrvIndex1 = 0;
UINT mNullTexSrvIndex2 = 0;
CD3DX12_GPU_DESCRIPTOR_HANDLE mNullSrv;
std::unique_ptr<ShadowMap> mShadowMap;
std::unique_ptr<Ssao> mSsao;

float mLightRotationAngle = 0.0f;
DirectX::XMFLOAT3 mBaseLightDirections[3] = {
	DirectX::XMFLOAT3(0.57735f, -0.57735f, 0.57735f),
	DirectX::XMFLOAT3(-0.57735f, -0.57735f, 0.57735f),
	DirectX::XMFLOAT3(0.0f, -0.707f, -0.707f)
};
DirectX::XMFLOAT3 mRotatedLightDirections[3];
std::string mSkinnedModelFilename = "soldier";
struct SkinnedModelInstance
{
	SkinnedData* SkinnedInfo = nullptr;
	std::vector<DirectX::XMFLOAT4X4> FinalTransforms;
	std::string ClipName;
	float TimePos = 0.0f;

	// Called every frame and increments the time position, interpolates the 
	// animations for each bone based on the current animation clip, and 
	// generates the final transforms which are ultimately set to the effect
	// for processing in the vertex shader.
	void UpdateSkinnedAnimation(float dt)
	{
		TimePos += dt;

		// Loop animation
		if (TimePos > SkinnedInfo->GetClipEndTime(ClipName))
			TimePos = 0.0f;

		// Compute the final transforms for this time position.
		SkinnedInfo->GetFinalTransforms(ClipName, TimePos, FinalTransforms);
	}
};

UINT mSkinnedSrvHeapStart = 0;
std::unique_ptr<SkinnedModelInstance> mSkinnedModelInst;
SkinnedData mSkinnedInfo;
std::vector<M3DLoader::Subset> mSkinnedSubsets;
std::vector<M3DLoader::M3dMaterial> mSkinnedMats;
std::vector<std::string> mSkinnedTextureNames;

DX12RHI::DX12RHI()
	: mCurrentFence(1), mCurrBackBuffer(0), mRtvDescriptorSize(0), m_DsvDescriptorSize(0), mCbvSrvUavDescriptorSize(0),
	m_PrespectiveCamera(CameraType::Perspective), m_OrthoGraphicCamera(CameraType::OrthoGraphic)
{
}

DX12RHI::~DX12RHI()
{
	if (md3dDevice)
	{
		FlushCommandQueue();
	}
}

void DX12RHI::Initialize(const RHIInitializeParam& param)
{
	m_InitParam = param;

	/*m_PrespectiveCamera.LookAt(DirectX::XMFLOAT3(5.0f, 4.0f, -15.0f),
		DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),
		DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));*/
	mSceneBounds.Center = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	mSceneBounds.Radius = sqrtf(10.0f * 10.0f + 15.0f * 15.0f);

	InitializeBase(param);

	ThrowIfFailed(mCommandList->Reset(m_CommandAllocator.Get(), nullptr));

	m_PrespectiveCamera.SetPosition(0.0f, 2.0f, -15.0f);

	{
		mShadowMap = std::make_unique<ShadowMap>(md3dDevice.Get(), 2048, 2048);
		mSsao = std::make_unique<Ssao>(md3dDevice.Get(), mCommandList.Get(), param.WindowWidth, param.WindowHeight);

		//DX12GeometryGenerator::Get()->Initialize();
		LoadSkinnedModel();
		LoadTextures();
		InitBaseRootSignatures();
		InitDescriptorHeaps();
		InitBaseShaders();
		BuildShapeGeometry();
		InitBaseMaterials();
		InitBaseRenderItems();
		InitFrameBuffer();
		InitBasePSOs();

		//mSsao->SetPSOs(mPSOs["ssao"].Get(), mPSOs["ssaoBlur"].Get());
		mSsao->SetPSOs(m_PSOs["ssao"]->GetPSO(), m_PSOs["ssaoBlur"]->GetPSO());
	}

	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	FlushCommandQueue();
}

void DX12RHI::OnUpdate(const RiverTime& time)
{
	m_PrespectiveCamera.OnUpdate();

	mLightRotationAngle += 0.1f * time.DeltaTime();

	XMMATRIX R = XMMatrixRotationY(mLightRotationAngle);
	for (int i = 0; i < 3; ++i)
	{
		XMVECTOR lightDir = XMLoadFloat3(&mBaseLightDirections[i]);
		lightDir = XMVector3TransformNormal(lightDir, R);
		XMStoreFloat3(&mRotatedLightDirections[i], lightDir);
	}

	UpdateShadowTransform(time);

	mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % s_FrameBufferCount;
	mCurrFrameResource = mFrameBuffer[mCurrFrameResourceIndex].get();
	if (mCurrFrameResource->m_FenceValue != 0 && mFence->GetCompletedValue() < mCurrFrameResource->m_FenceValue)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

		ThrowIfFailed(mFence->SetEventOnCompletion(mCurrFrameResource->m_FenceValue, eventHandle));

		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	AnimationMaterials(time);
	UpdateObjectCBs();
	UpdateSkinnedCBs(time);
	UpdateMaterialCBs();
	//UpdateShadowTransform(time);
	UpdateMainPass(time);
	UpdateShadowPass(time);
	UpdateSsaoCBs(time);
}

void DX12RHI::UpdateUIData(V_Array<UIVertex>& vertices, V_Array<uint32_t> indices)
{
	//mCurrFrameResource->Update
}

DX12Texture* DX12RHI::CreateTexture(const char* name, const char* filePath)
{
	decltype(DX12RHI::CreateTexture(name, filePath)) ret = nullptr;
	if (name && filePath && mTextures.find(name) == std::end(mTextures))
	{
		auto texture =  MakeUnique<DX12Texture>(md3dDevice.Get(), mCommandList.Get(), name, filePath);
		ret = texture.get();
		mTextures[name] = std::move(texture);
	}

	return ret;
}

void DX12RHI::Render()
{
	auto cmdListAlloc = mCurrFrameResource->m_CommandAlloc;

	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(cmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	//ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mPSOs["opaque"].Get()));
	ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), m_PSOs["opaque"]->GetPSO()));

	ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvDescriptorHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mCommandList->SetGraphicsRootSignature(m_RootSignatures["default"]->GetRootSignature());

	//
	// Shadow map pass.
	//

	// Bind all the materials used in this scene.  For structured buffers, we can bypass the heap and 
	// set as a root descriptor.
	auto matBuffer = mCurrFrameResource->m_MaterialUniform->Resource();
	mCommandList->SetGraphicsRootShaderResourceView(3, matBuffer->GetGPUVirtualAddress());

	// Bind null SRV for shadow map pass.
	mCommandList->SetGraphicsRootDescriptorTable(4, mNullSrv);

	// Bind all the textures used in this scene.  Observe
	// that we only have to specify the first descriptor in the table.  
	// The root signature knows how many descriptors are expected in the table.
	mCommandList->SetGraphicsRootDescriptorTable(5, mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	DrawSceneToShadowMap();

	//
	// Normal/depth pass.
	//

	DrawNormalsAndDepth();

	//
	//
	// 

	mCommandList->SetGraphicsRootSignature(m_RootSignatures["ssao"]->GetRootSignature());
	mSsao->ComputeSsao(mCommandList.Get(), mCurrFrameResource, 2);

	//
	// Main rendering pass.
	//

	mCommandList->SetGraphicsRootSignature(m_RootSignatures["default"]->GetRootSignature());

	// Rebind state whenever graphics root signature changes.

	// Bind all the materials used in this scene.  For structured buffers, we can bypass the heap and 
	// set as a root descriptor.
	matBuffer = mCurrFrameResource->m_MaterialUniform->Resource();
	mCommandList->SetGraphicsRootShaderResourceView(3, matBuffer->GetGPUVirtualAddress());


	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Clear the back buffer and depth buffer.
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	// Bind all the textures used in this scene.  Observe
	// that we only have to specify the first descriptor in the table.  
	// The root signature knows how many descriptors are expected in the table.
	mCommandList->SetGraphicsRootDescriptorTable(5, mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	auto passCB = mCurrFrameResource->m_PassUniform->Resource();
	mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	// Bind the sky cube map.  For our demos, we just use one "world" cube map representing the environment
	// from far away, so all objects will use the same cube map and we only need to set it once per-frame.  
	// If we wanted to use "local" cube maps, we would have to change them per-object, or dynamically
	// index into an array of cube maps.

	CD3DX12_GPU_DESCRIPTOR_HANDLE skyTexDescriptor(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	skyTexDescriptor.Offset(mSkyTexHeapIndex, mCbvSrvUavDescriptorSize);
	mCommandList->SetGraphicsRootDescriptorTable(4, skyTexDescriptor);

	//mCommandList->SetPipelineState(mPSOs["opaque"].Get());
	mCommandList->SetPipelineState(m_PSOs["opaque"]->GetPSO());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Opaque]);

	//mCommandList->SetPipelineState(mPSOs["skinnedOpaque"].Get());
	mCommandList->SetPipelineState(m_PSOs["skinnedOpaque"]->GetPSO());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::SkinnedOpaque]);

	//mCommandList->SetPipelineState(mPSOs["debug"].Get());
	mCommandList->SetPipelineState(m_PSOs["debug"]->GetPSO());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Debug]);

	//mCommandList->SetPipelineState(mPSOs["sky"].Get());
	mCommandList->SetPipelineState(m_PSOs["sky"]->GetPSO());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Sky]);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// Done recording commands.
	ThrowIfFailed(mCommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// Advance the fence value to mark commands up to this fence point.
	mCurrFrameResource->m_FenceValue = ++mCurrentFence;

	// Add an instruction to the command queue to set a new fence point. 
	// Because we are on the GPU timeline, the new fence point won't be 
	// set until the GPU finishes processing all the commands prior to this Signal().
	mCommandQueue->Signal(mFence.Get(), mCurrentFence);
}

Unique<DX12PipelineState> DX12RHI::CreatePSO(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc, const V_Array<D3D12_INPUT_ELEMENT_DESC>* layout, Shader* vsShader, Shader* psShader)
{
	if (layout)
	{
		desc.InputLayout = { layout->data(), (UINT)layout->size() };
	}

	if (vsShader)
	{
		auto dx12VsShader = dynamic_cast<DX12Shader*>(vsShader);
		desc.VS = { GetShaderBufferPointer(dx12VsShader), GetShaderBufferSize(dx12VsShader) };
	}

	if (psShader)
	{
		auto dx12PsShader = dynamic_cast<DX12Shader*>(psShader);
		desc.PS = { GetShaderBufferPointer(dx12PsShader), GetShaderBufferSize(dx12PsShader) };
	}

	return MakeUnique<DX12PipelineState>(md3dDevice.Get(), desc);
}

Unique<DX12VertexBuffer> DX12RHI::CreateVertexBuffer(float* vertices, uint32_t byteSize, uint32_t elementSize, const V_Array<D3D12_INPUT_ELEMENT_DESC>* layout)
{
	return MakeUnique<DX12VertexBuffer>(md3dDevice.Get(), mCommandList.Get(), vertices, byteSize, elementSize, layout);
}

Unique<DX12IndexBuffer> DX12RHI::CreateIndexBuffer(void* indices, uint32_t count, ShaderDataType indiceDataType)
{
	return MakeUnique<DX12IndexBuffer>(md3dDevice.Get(), mCommandList.Get(), indices, count, indiceDataType);
}

void DX12RHI::Resize(const RHIInitializeParam& param)
{
	FlushCommandQueue();

	ThrowIfFailed(mCommandList->Reset(m_CommandAllocator.Get(), nullptr));

	for (size_t i = 0; i < SwapChainBufferCount; i++)
	{
		m_SwapChainBuffer[i].Reset();
	}
	mDepthStencilBuffer.Reset();

	ThrowIfFailed(mSwapChain->ResizeBuffers(SwapChainBufferCount, param.WindowWidth, param.WindowHeight,
		mBackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	mCurrBackBuffer = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < SwapChainBufferCount; i++)
	{
		ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_SwapChainBuffer[i])));
		md3dDevice->CreateRenderTargetView(m_SwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, mRtvDescriptorSize);
	}

	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = param.WindowWidth;
	depthStencilDesc.Height = param.WindowHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthStencilDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = mDepthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	auto heapDefault = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	md3dDevice->CreateCommittedResource(&heapDefault, D3D12_HEAP_FLAG_NONE, &depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON, &optClear, IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf()));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = mDepthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;
	md3dDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

	auto sb_2w = CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	mCommandList->ResourceBarrier(1, &sb_2w);

	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	FlushCommandQueue();

	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width = static_cast<float>(param.WindowWidth);
	mScreenViewport.Height = static_cast<float>(param.WindowHeight);
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;

	mScissorRect = { 0, 0, param.WindowWidth, param.WindowHeight };

	m_PrespectiveCamera.SetLens(0.25f * PI, (float)param.WindowWidth / param.WindowHeight, 1.0f, 1000.0f);
	if (mSsao != nullptr)
	{
		mSsao->OnResize(param.WindowWidth, param.WindowHeight);

		mSsao->RebuildDescriptors(mDepthStencilBuffer.Get());
	}
}

void DX12RHI::InitializeBase(const RHIInitializeParam& param)
{
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_Factory)));

	EnumAdaptersAndCreateDevice();

	CreateFence();

	mRtvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_DsvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	mCbvSrvUavDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	CheckQualityLevel();

	CreateCommandQueue();

	CreateSwapChain();

	CreateRtvAndDsvHeaps();

	Resize(param);
}

void DX12RHI::LoadSkinnedModel()
{
	std::vector<M3DLoader::DX12SkinnedVertex> vertices;
	std::vector<std::uint16_t> indices;

	M3DLoader m3dLoader;
	m3dLoader.LoadM3d(DEFAULT_MODEL_PATH_3, vertices, indices,
		mSkinnedSubsets, mSkinnedMats, mSkinnedInfo);

	mSkinnedModelInst = std::make_unique<SkinnedModelInstance>();
	mSkinnedModelInst->SkinnedInfo = &mSkinnedInfo;
	mSkinnedModelInst->FinalTransforms.resize(mSkinnedInfo.BoneCount());
	mSkinnedModelInst->ClipName = "Take1";
	mSkinnedModelInst->TimePos = 0.0f;

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(DX12SkinnedVertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = mSkinnedModelFilename;
	geo->CopyCPUData(vertices, indices);
	geo->SetVertexBufferAndIndexBuffer(CreateVertexBuffer((float*)vertices.data(), vbByteSize, (UINT)sizeof(DX12SkinnedVertex), &m_InputLayers["skinnedDefault"]),
		CreateIndexBuffer(indices.data(), (UINT)indices.size(), ShaderDataType::Short));

	for (UINT i = 0; i < (UINT)mSkinnedSubsets.size(); ++i)
	{
		SubmeshGeometry submesh;
		std::string name = "sm_" + std::to_string(i);

		submesh.IndexCount = (UINT)mSkinnedSubsets[i].FaceCount * 3;
		submesh.StartIndexLocation = mSkinnedSubsets[i].FaceStart * 3;
		submesh.BaseVertexLocation = 0;

		geo->DrawArgs[name] = submesh;
	}

	mGeometries[geo->Name] = std::move(geo);
}

void DX12RHI::BuildShapeGeometry()
{
	DX12GeometryGenerator geoGen;
	DX12GeometryGenerator::MeshData box = geoGen.CreateBox1(1.0f, 1.0f, 1.0f, 3);
	DX12GeometryGenerator::MeshData grid = geoGen.CreateGrid1(20.0f, 30.0f, 60, 40);
	DX12GeometryGenerator::MeshData sphere = geoGen.CreateSphere1(0.5f, 20, 20);
	DX12GeometryGenerator::MeshData cylinder = geoGen.CreateCylinder1(0.5f, 0.3f, 3.0f, 20, 20);
	DX12GeometryGenerator::MeshData quad = geoGen.CreateQuad1(0.0f, 0.0f, 1.0f, 1.0f, 0.0f);

	UINT boxVertexOffset = 0;
	UINT gridVertexOffset = (UINT)box.Vertices.size();
	UINT sphereVertexOffset = gridVertexOffset + (UINT)grid.Vertices.size();
	UINT cylinderVertexOffset = sphereVertexOffset + (UINT)sphere.Vertices.size();
	UINT quadVertexOffset = cylinderVertexOffset + (UINT)cylinder.Vertices.size();

	UINT boxIndexOffset = 0;
	UINT gridIndexOffset = (UINT)box.Indices32.size();
	UINT sphereIndexOffset = gridIndexOffset + (UINT)grid.Indices32.size();
	UINT cylinderIndexOffset = sphereIndexOffset + (UINT)sphere.Indices32.size();
	UINT quadIndexOffset = cylinderIndexOffset + (UINT)cylinder.Indices32.size();

	SubmeshGeometry boxSubmesh;
	boxSubmesh.IndexCount = (UINT)box.Indices32.size();
	boxSubmesh.StartIndexLocation = boxIndexOffset;
	boxSubmesh.BaseVertexLocation = boxVertexOffset;

	SubmeshGeometry gridSubmesh;
	gridSubmesh.IndexCount = (UINT)grid.Indices32.size();
	gridSubmesh.StartIndexLocation = gridIndexOffset;
	gridSubmesh.BaseVertexLocation = gridVertexOffset;

	SubmeshGeometry sphereSubmesh;
	sphereSubmesh.IndexCount = (UINT)sphere.Indices32.size();
	sphereSubmesh.StartIndexLocation = sphereIndexOffset;
	sphereSubmesh.BaseVertexLocation = sphereVertexOffset;

	SubmeshGeometry cylinderSubmesh;
	cylinderSubmesh.IndexCount = (UINT)cylinder.Indices32.size();
	cylinderSubmesh.StartIndexLocation = cylinderIndexOffset;
	cylinderSubmesh.BaseVertexLocation = cylinderVertexOffset;

	SubmeshGeometry quadSubmesh;
	quadSubmesh.IndexCount = (UINT)quad.Indices32.size();
	quadSubmesh.StartIndexLocation = quadIndexOffset;
	quadSubmesh.BaseVertexLocation = quadVertexOffset;

	auto totalVertexCount =
		box.Vertices.size() +
		grid.Vertices.size() +
		sphere.Vertices.size() +
		cylinder.Vertices.size() +
		quad.Vertices.size();

	std::vector<DX12Vertex> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].TexC = box.Vertices[i].TexC;
		vertices[k].TangentU = box.Vertices[i].TangentU;
	}

	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Normal = grid.Vertices[i].Normal;
		vertices[k].TexC = grid.Vertices[i].TexC;
		vertices[k].TangentU = grid.Vertices[i].TangentU;
	}

	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		vertices[k].Normal = sphere.Vertices[i].Normal;
		vertices[k].TexC = sphere.Vertices[i].TexC;
		vertices[k].TangentU = sphere.Vertices[i].TangentU;
	}

	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		vertices[k].Normal = cylinder.Vertices[i].Normal;
		vertices[k].TexC = cylinder.Vertices[i].TexC;
		vertices[k].TangentU = cylinder.Vertices[i].TangentU;
	}

	for (int i = 0; i < quad.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = quad.Vertices[i].Position;
		vertices[k].Normal = quad.Vertices[i].Normal;
		vertices[k].TexC = quad.Vertices[i].TexC;
		vertices[k].TangentU = quad.Vertices[i].TangentU;
	}

	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
	indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));
	indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
	indices.insert(indices.end(), std::begin(cylinder.GetIndices16()), std::end(cylinder.GetIndices16()));
	indices.insert(indices.end(), std::begin(quad.GetIndices16()), std::end(quad.GetIndices16()));

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(DX12Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "shapeGeo";
	geo->CopyCPUData(vertices, indices);
	geo->SetVertexBufferAndIndexBuffer(CreateVertexBuffer((float*)vertices.data(), vbByteSize, (UINT)sizeof(DX12Vertex), &m_InputLayers["skinnedDefault"]),
		CreateIndexBuffer(indices.data(), (UINT)indices.size(), ShaderDataType::Short));

	geo->DrawArgs["box"] = boxSubmesh;
	geo->DrawArgs["grid"] = gridSubmesh;
	geo->DrawArgs["sphere"] = sphereSubmesh;
	geo->DrawArgs["cylinder"] = cylinderSubmesh;
	geo->DrawArgs["quad"] = quadSubmesh;

	mGeometries[geo->Name] = std::move(geo);
}

void DX12RHI::LoadTextures()
{
	CreateTexture("bricksDiffuseMap", DEFAULT_TEXTURE_PATH_13);
	CreateTexture("bricksNormalMap", DEFAULT_TEXTURE_PATH_14);
	CreateTexture("tileDiffuseMap", DEFAULT_TEXTURE_PATH_3);
	CreateTexture("tileNormalMap", DEFAULT_TEXTURE_PATH_15);
	CreateTexture("defaultDiffuseMap", DEFAULT_TEXTURE_PATH_10);
	CreateTexture("defaultNormalMap", DEFAULT_TEXTURE_PATH_16);
	CreateTexture("skyCubeMap", DEFAULT_TEXTURE_PATH_18);

	for (UINT i = 0; i < mSkinnedMats.size(); ++i)
	{
		std::string diffuseName = mSkinnedMats[i].DiffuseMapName;
		std::string normalName = mSkinnedMats[i].NormalMapName;

		std::string diffuseFilename = DEFAULT_TEXTURE_PATH + diffuseName;
		std::string normalFilename = DEFAULT_TEXTURE_PATH + normalName;

		diffuseName = diffuseName.substr(0, diffuseName.find_last_of("."));
		normalName = normalName.substr(0, normalName.find_last_of("."));

		CreateTexture(diffuseName.c_str(), diffuseFilename.c_str());
		CreateTexture(normalName.c_str(), normalFilename.c_str());

		mSkinnedTextureNames.push_back(diffuseName);
		mSkinnedTextureNames.push_back(normalName);
	}
}

void DX12RHI::InitBaseMaterials()
{
	auto bricks = MakeUnique<Material>("bricks0");
	bricks->InitBaseParam({ 1.0f, 1.0f, 1.0f, 1.0f }, { 0.1f, 0.1f, 0.1f }, 0.3f, 0, 0, 1);
	auto tile = MakeUnique<Material>("tile0");
	tile->InitBaseParam({ 0.9f, 0.9f, 0.9f, 1.0f }, { 0.2f, 0.2f, 0.2f }, 0.1f, 1, 2, 3);
	auto mirror = MakeUnique<Material>("mirror0");
	mirror->InitBaseParam({ 0.0f, 0.0f, 0.0f, 1.0f }, { 0.98f, 0.97f, 0.95f }, 0.1f, 2, 4, 5);
	auto sky = MakeUnique<Material>("sky");
	sky->InitBaseParam({ 1.0f, 1.0f, 1.0f, 1.0f }, { 0.1f, 0.1f, 0.1f }, 1.0f, 3, 6, 7);

	mMaterials["bricks0"] = std::move(bricks);
	mMaterials["tile0"] = std::move(tile);
	mMaterials["mirror0"] = std::move(mirror);
	mMaterials["sky"] = std::move(sky);

	UINT matCBIndex = 4;
	UINT srvHeapIndex = mSkinnedSrvHeapStart;
	for (UINT i = 0; i < mSkinnedMats.size(); ++i)
	{
		int DiffuseSrvHeapIndex = srvHeapIndex++;
		int NormalSrvHeapIndex = srvHeapIndex++;
		auto mat = MakeUnique<Material>(mSkinnedMats[i].Name.c_str());
		mat->InitBaseParam(*(River::Float4*)(&mSkinnedMats[i].DiffuseAlbedo), *(River::Float3*)(&mSkinnedMats[i].FresnelR0), mSkinnedMats[i].Roughness,
			matCBIndex++, DiffuseSrvHeapIndex, NormalSrvHeapIndex);
		mMaterials[mSkinnedMats[i].Name] = std::move(mat);
	}
}

void DX12RHI::InitBaseShaders()
{
	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"ALPHA_TEST", "1",
		NULL, NULL
	};

	const D3D_SHADER_MACRO skinnedDefines[] =
	{
		"SKINNED", "1",
		NULL, NULL
	};

	mShaders["standardVS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_1, nullptr, "VS", "vs_5_1");
	mShaders["skinnedVS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_1, skinnedDefines, "VS", "vs_5_1");
	mShaders["opaquePS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_1, nullptr, "PS", "ps_5_1");

	mShaders["shadowVS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_3, nullptr, "VS", "vs_5_1");
	mShaders["skinnedShadowVS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_3, skinnedDefines, "VS", "vs_5_1");
	mShaders["shadowOpaquePS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_3, nullptr, "PS", "ps_5_1");
	mShaders["shadowAlphaTestedPS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_3, alphaTestDefines, "PS", "ps_5_1");

	mShaders["debugVS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_4, nullptr, "VS", "vs_5_1"); 
	mShaders["debugPS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_4, nullptr, "PS", "ps_5_1"); 

	mShaders["drawNormalsVS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_6, nullptr, "VS", "vs_5_1"); 
	mShaders["skinnedDrawNormalsVS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_6, skinnedDefines, "VS", "vs_5_1"); 
	mShaders["drawNormalsPS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_6, nullptr, "PS", "ps_5_1"); 

	mShaders["ssaoVS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_5, nullptr, "VS", "vs_5_1"); 
	mShaders["ssaoPS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_5, nullptr, "PS", "ps_5_1"); 

	mShaders["ssaoBlurVS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_7, nullptr, "VS", "vs_5_1"); 
	mShaders["ssaoBlurPS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_7, nullptr, "PS", "ps_5_1"); 

	mShaders["skyVS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_2, nullptr, "VS", "vs_5_1"); 
	mShaders["skyPS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_2, nullptr, "PS", "ps_5_1"); 

	m_InputLayers["default"] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	//mInputLayout = m_InputLayers["default"];
	

	m_InputLayers["skinnedDefault"] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BONEINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 56, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	//mSkinnedInputLayout = m_InputLayers["skinnedDefault"];
}

void DX12RHI::InitBaseRootSignatures()
{
	{
		CD3DX12_DESCRIPTOR_RANGE texTable0;
		texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0, 0);

		CD3DX12_DESCRIPTOR_RANGE texTable1;
		texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 48, 3, 0);

		CD3DX12_ROOT_PARAMETER slotRootParameter[6];

		// Perfomance TIP: Order from most frequent to least frequent.
		slotRootParameter[0].InitAsConstantBufferView(0);
		slotRootParameter[1].InitAsConstantBufferView(1);
		slotRootParameter[2].InitAsConstantBufferView(2);
		slotRootParameter[3].InitAsShaderResourceView(0, 1);
		slotRootParameter[4].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter[5].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);

		auto staticSamplers = DX12RootSignature::GetStaticSamplers();

		// A root signature is an array of root parameters.
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(6, slotRootParameter,
			(UINT)staticSamplers.size(), staticSamplers.data(),
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		m_RootSignatures["default"] = MakeUnique<DX12RootSignature>(md3dDevice.Get(), rootSigDesc);
	}

	{
		CD3DX12_DESCRIPTOR_RANGE texTable0;
		texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0);

		CD3DX12_DESCRIPTOR_RANGE texTable1;
		texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0);

		// Root parameter can be a table, root descriptor or root constants.
		CD3DX12_ROOT_PARAMETER slotRootParameter[4];

		// Perfomance TIP: Order from most frequent to least frequent.
		slotRootParameter[0].InitAsConstantBufferView(0);
		slotRootParameter[1].InitAsConstants(1, 1);
		slotRootParameter[2].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter[3].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);

		const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
			0, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
			1, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC depthMapSam(
			2, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
			D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
			0.0f,
			0,
			D3D12_COMPARISON_FUNC_LESS_EQUAL,
			D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE);

		const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
			3, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

		std::array<CD3DX12_STATIC_SAMPLER_DESC, 4> staticSamplers =
		{
			pointClamp, linearClamp, depthMapSam, linearWrap
		};

		// A root signature is an array of root parameters.
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter,
			(UINT)staticSamplers.size(), staticSamplers.data(),
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		m_RootSignatures["ssao"] = MakeUnique<DX12RootSignature>(md3dDevice.Get(), rootSigDesc);
	}
}

void DX12RHI::InitBasePSOs()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.pRootSignature = m_RootSignatures["default"]->GetRootSignature();
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = mBackBufferFormat;
	opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = mDepthStencilFormat;
	m_PSOs["opaque"] = CreatePSO(opaquePsoDesc, &m_InputLayers["default"], mShaders["standardVS"].get(), mShaders["opaquePS"].get());

	D3D12_GRAPHICS_PIPELINE_STATE_DESC skinnedOpaquePsoDesc = opaquePsoDesc;
	m_PSOs["skinnedOpaque"] = CreatePSO(skinnedOpaquePsoDesc, &m_InputLayers["skinnedDefault"], mShaders["skinnedVS"].get(), mShaders["opaquePS"].get());

	D3D12_GRAPHICS_PIPELINE_STATE_DESC smapPsoDesc = opaquePsoDesc;
	smapPsoDesc.RasterizerState.DepthBias = 100000;
	smapPsoDesc.RasterizerState.DepthBiasClamp = 0.0f;
	smapPsoDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;
	smapPsoDesc.pRootSignature = m_RootSignatures["default"]->GetRootSignature();

	// Shadow map pass does not have a render target.
	smapPsoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
	smapPsoDesc.NumRenderTargets = 0;
	m_PSOs["shadow_opaque"] = CreatePSO(smapPsoDesc, nullptr, mShaders["shadowVS"].get(), mShaders["shadowOpaquePS"].get());

	D3D12_GRAPHICS_PIPELINE_STATE_DESC skinnedSmapPsoDesc = smapPsoDesc;
	m_PSOs["skinnedShadow_opaque"] = CreatePSO(skinnedSmapPsoDesc, &m_InputLayers["skinnedDefault"], mShaders["skinnedShadowVS"].get(), mShaders["shadowOpaquePS"].get());

	D3D12_GRAPHICS_PIPELINE_STATE_DESC debugPsoDesc = opaquePsoDesc;
	debugPsoDesc.pRootSignature = m_RootSignatures["default"]->GetRootSignature();
	m_PSOs["debug"] = CreatePSO(debugPsoDesc, nullptr, mShaders["debugVS"].get(), mShaders["debugPS"].get());

	D3D12_GRAPHICS_PIPELINE_STATE_DESC drawNormalsPsoDesc = opaquePsoDesc;
	drawNormalsPsoDesc.RTVFormats[0] = Ssao::NormalMapFormat;
	drawNormalsPsoDesc.SampleDesc.Count = 1;
	drawNormalsPsoDesc.SampleDesc.Quality = 0;
	drawNormalsPsoDesc.DSVFormat = mDepthStencilFormat;
	m_PSOs["drawNormals"] = CreatePSO(drawNormalsPsoDesc, nullptr, mShaders["drawNormalsVS"].get(), mShaders["drawNormalsPS"].get());

	D3D12_GRAPHICS_PIPELINE_STATE_DESC skinnedDrawNormalsPsoDesc = drawNormalsPsoDesc;
	m_PSOs["skinnedDrawNormals"] = CreatePSO(skinnedDrawNormalsPsoDesc, &m_InputLayers["skinnedDefault"], mShaders["skinnedDrawNormalsVS"].get(), mShaders["drawNormalsPS"].get());

	D3D12_GRAPHICS_PIPELINE_STATE_DESC ssaoPsoDesc = opaquePsoDesc;
	ssaoPsoDesc.InputLayout = { nullptr, 0 };
	ssaoPsoDesc.pRootSignature = m_RootSignatures["ssao"]->GetRootSignature();

	// SSAO effect does not need the depth buffer.
	ssaoPsoDesc.DepthStencilState.DepthEnable = false;
	ssaoPsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	ssaoPsoDesc.RTVFormats[0] = Ssao::AmbientMapFormat;
	ssaoPsoDesc.SampleDesc.Count = 1;
	ssaoPsoDesc.SampleDesc.Quality = 0;
	ssaoPsoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
	m_PSOs["ssao"] = CreatePSO(ssaoPsoDesc, nullptr, mShaders["ssaoVS"].get(), mShaders["ssaoPS"].get());

	D3D12_GRAPHICS_PIPELINE_STATE_DESC ssaoBlurPsoDesc = ssaoPsoDesc;
	m_PSOs["ssaoBlur"] = CreatePSO(ssaoBlurPsoDesc, nullptr, mShaders["ssaoBlurVS"].get(), mShaders["ssaoBlurPS"].get());

	D3D12_GRAPHICS_PIPELINE_STATE_DESC skyPsoDesc = opaquePsoDesc;

	// The camera is inside the sky sphere, so just turn off culling.
	skyPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	// Make sure the depth function is LESS_EQUAL and not just LESS.  
	// Otherwise, the normalized depth values at z = 1 (NDC) will 
	// fail the depth test if the depth buffer was cleared to 1.
	skyPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	skyPsoDesc.pRootSignature = m_RootSignatures["default"]->GetRootSignature();
	m_PSOs["sky"] = CreatePSO(skyPsoDesc, nullptr, mShaders["skyVS"].get(), mShaders["skyPS"].get());
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

	ThrowIfFailed(D3D12CreateDevice(pWarpAdapter.Get(), emFeatureLevel, IID_PPV_ARGS(&md3dDevice)));
}

void DX12RHI::CreateCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC stQueueDesc = {};
	stQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	stQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(md3dDevice->CreateCommandQueue(&stQueueDesc, IID_PPV_ARGS(&mCommandQueue)));

	ThrowIfFailed(md3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_CommandAllocator.GetAddressOf())));

	ThrowIfFailed(md3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(),
		nullptr, IID_PPV_ARGS(mCommandList.GetAddressOf())));

	mCommandList->Close();
}

void DX12RHI::CreateSwapChain()
{
	mSwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = m_InitParam.WindowWidth;
	sd.BufferDesc.Height = m_InitParam.WindowHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = mBackBufferFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	sd.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaState - 1) : 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = SwapChainBufferCount;
	sd.OutputWindow = (HWND)m_InitParam.HWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ThrowIfFailed(m_Factory->CreateSwapChain(mCommandQueue.Get(), &sd, mSwapChain.GetAddressOf()));
}

Camera* DX12RHI::GetMainCamera()
{
	return &m_PrespectiveCamera;
}

void DX12RHI::Pick(int x, int y)
{
	DirectX::XMFLOAT4X4 P = m_PrespectiveCamera.m_Proj;

	// Compute picking ray in view space.
	float vx = (+2.0f * x / m_InitParam.WindowWidth - 1.0f) / P(0, 0);
	float vy = (-2.0f * y / m_InitParam.WindowHeight + 1.0f) / P(1, 1);

	// Ray definition in view space.
	DirectX::XMVECTOR rayOrigin = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	DirectX::XMVECTOR rayDir = DirectX::XMVectorSet(vx, vy, 1.0f, 0.0f);

	DirectX::XMMATRIX V = m_PrespectiveCamera.GetView();
	auto dv = XMMatrixDeterminant(V);
	DirectX::XMMATRIX invView = XMMatrixInverse(&dv, V);

	// Assume nothing is picked to start, so the picked render-item is invisible.
	mPickedRitem->Visible = false;

	// Check if we picked an opaque render item.  A real app might keep a separate "picking list"
	// of objects that can be selected.   
	for (auto ri : mRitemLayer[(int)RenderLayer::Opaque])
	{
		auto geo = ri->Geo;

		// Skip invisible render-items.
		/*if (ri->Visible == false)
			continue;*/

		DirectX::XMMATRIX W = XMLoadFloat4x4(&ri->World);
		auto dw = XMMatrixDeterminant(W);
		DirectX::XMMATRIX invWorld = XMMatrixInverse(&dw, W);

		// Tranform ray to vi space of Mesh.
		DirectX::XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);

		rayOrigin = XMVector3TransformCoord(rayOrigin, toLocal);
		rayDir = XMVector3TransformNormal(rayDir, toLocal);

		// Make the ray direction unit length for the intersection tests.
		rayDir = DirectX::XMVector3Normalize(rayDir);

		// If we hit the bounding box of the Mesh, then we might have picked a Mesh triangle,
		// so do the ray/triangle tests.
		//
		// If we did not hit the bounding box, then it is impossible that we hit 
		// the Mesh, so do not waste effort doing ray/triangle tests.
		float tmin = 0.0f;
		//if (ri->Bounds.Intersects(rayOrigin, rayDir, tmin))
		{
			// NOTE: For the demo, we know what to cast the vertex/index data to.  If we were mixing
			// formats, some metadata would be needed to figure out what to cast it to.
			auto vertices = (DX12Vertex*)geo->VertexBufferCPU->GetBufferPointer();
			auto indices = (std::uint32_t*)geo->IndexBufferCPU->GetBufferPointer();
			UINT triCount = ri->IndexCount / 3;

			// Find the nearest ray/triangle intersection.
			tmin = FLT_MAX;
			for (UINT i = 0; i < triCount; ++i)
			{
				// Indices for this triangle.
				UINT i0 = indices[i * 3 + 0];
				UINT i1 = indices[i * 3 + 1];
				UINT i2 = indices[i * 3 + 2];

				// Vertices for this triangle.
				DirectX::XMVECTOR v0 = XMLoadFloat3(&vertices[i0].Pos);
				DirectX::XMVECTOR v1 = XMLoadFloat3(&vertices[i1].Pos);
				DirectX::XMVECTOR v2 = XMLoadFloat3(&vertices[i2].Pos);

				// We have to iterate over all the triangles in order to find the nearest intersection.
				float t = 0.0f;
				if (DirectX::TriangleTests::Intersects(rayOrigin, rayDir, v0, v1, v2, t))
				{
					if (t < tmin)
					{
						// This is the new nearest picked triangle.
						tmin = t;
						UINT pickedTriangle = i;

						mPickedRitem->Visible = true;
						mPickedRitem->IndexCount = 3;
						mPickedRitem->BaseVertexLocation = 0;

						// Picked render item needs same world matrix as object picked.
						mPickedRitem->World = ri->World;
						mPickedRitem->NumFramesDirty = 3;

						// Offset to the picked triangle in the mesh index buffer.
						mPickedRitem->StartIndexLocation = 3 * pickedTriangle;
					}
				}
			}
		}
	}
}

void DX12RHI::CreateRtvAndDsvHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC stRTVHeapDesc = {};
	stRTVHeapDesc.NumDescriptors = SwapChainBufferCount + 3;
	stRTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	stRTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	stRTVHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&stRTVHeapDesc, IID_PPV_ARGS(mRtvDescriptorHeap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 2;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(m_DsvHeap.GetAddressOf())));
}

void DX12RHI::CreateFence()
{
	ThrowIfFailed(md3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));
}

void DX12RHI::CheckQualityLevel()
{
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = mBackBufferFormat;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	ThrowIfFailed(md3dDevice->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)));

	m4xMsaaQuality = msQualityLevels.NumQualityLevels;
	assert(m4xMsaaQuality > 0 && "Unexpected MSAA quality level.");
}

void DX12RHI::AnimationMaterials(const RiverTime& time)
{
}

void DX12RHI::UpdateShadowTransform(const RiverTime& time)
{
	// Only the first "main" light casts a shadow.
	XMVECTOR lightDir = XMLoadFloat3(&mRotatedLightDirections[0]);
	XMVECTOR lightPos = -2.0f * mSceneBounds.Radius * lightDir;
	XMVECTOR targetPos = XMLoadFloat3(&mSceneBounds.Center);
	XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX lightView = XMMatrixLookAtLH(lightPos, targetPos, lightUp);

	XMStoreFloat3(&mLightPosW, lightPos);

	// Transform bounding sphere to light space.
	XMFLOAT3 sphereCenterLS;
	XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, lightView));

	// Ortho frustum in light space encloses scene.
	float l = sphereCenterLS.x - mSceneBounds.Radius;
	float b = sphereCenterLS.y - mSceneBounds.Radius;
	float n = sphereCenterLS.z - mSceneBounds.Radius;
	float r = sphereCenterLS.x + mSceneBounds.Radius;
	float t = sphereCenterLS.y + mSceneBounds.Radius;
	float f = sphereCenterLS.z + mSceneBounds.Radius;

	mLightNearZ = n;
	mLightFarZ = f;
	XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX S = lightView * lightProj * T;
	XMStoreFloat4x4(&mLightView, lightView);
	XMStoreFloat4x4(&mLightProj, lightProj);
	XMStoreFloat4x4(&mShadowTransform, S);
}
void DX12RHI::UpdateObjectCBs()
{
	auto& currObjectCB = mCurrFrameResource->m_ObjectUniform;

	for (auto& e : mAllRitems)
	{
		if (e->NumFramesDirty > 0)
		{
			DirectX::XMMATRIX world = XMLoadFloat4x4(&e->World);
			DirectX::XMMATRIX texTransform = XMLoadFloat4x4(&e->TexTransform);

			ObjectUniform objConstants;
			XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(world));
			XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
			objConstants.MaterialIndex = e->Mat->MatCBIndex;

			currObjectCB->CopyData(e->ObjCBIndex, objConstants);

			// Next FrameResource need to be updated too.
			e->NumFramesDirty--;
		}
	}
}

void DX12RHI::UpdateSkinnedCBs(const RiverTime& time)
{
	auto currSkinnedCB = mCurrFrameResource->m_SkinnedUniform.get();

	// We only have one skinned model being animated.
	mSkinnedModelInst->UpdateSkinnedAnimation(time.DeltaTime());

	SkinnedUniform skinnedConstants;
	std::copy(
		std::begin(mSkinnedModelInst->FinalTransforms),
		std::end(mSkinnedModelInst->FinalTransforms),
		&skinnedConstants.BoneTransforms[0]);

	currSkinnedCB->CopyData(0, skinnedConstants);
}

void DX12RHI::UpdateMaterialCBs()
{
	auto currMaterialCB = mCurrFrameResource->m_MaterialUniform.get();
	for (auto& e : mMaterials)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		auto mat = e.second.get();
		if (mat->NumFramesDirty > 0)
		{
			XMMATRIX matTransform = DirectX::XMLoadFloat4x4((DirectX::XMFLOAT4X4*)(&mat->MatTransform));

			MaterialUniform matData;
			matData.DiffuseAlbedo = *(DirectX::XMFLOAT4*)(&mat->DiffuseAlbedo);
			matData.FresnelR0 = *(DirectX::XMFLOAT3*)(&mat->FresnelR0);
			matData.Roughness = mat->Roughness;
			DirectX::XMStoreFloat4x4((DirectX::XMFLOAT4X4*)(&mat->MatTransform), XMMatrixTranspose(matTransform));
			matData.DiffuseMapIndex = mat->DiffuseSrvHeapIndex;
			matData.NormalMapIndex = mat->NormalSrvHeapIndex;

			currMaterialCB->CopyData(mat->MatCBIndex, matData);

			// Next FrameResource need to be updated too.
			mat->NumFramesDirty--;
		}
	}
}

void DX12RHI::UpdateMainPass(const RiverTime& time)
{
	XMMATRIX view = m_PrespectiveCamera.GetView();
	XMMATRIX proj = m_PrespectiveCamera.GetProj();

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX viewProjTex = XMMatrixMultiply(viewProj, T);
	XMMATRIX shadowTransform = XMLoadFloat4x4(&mShadowTransform);

	XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	XMStoreFloat4x4(&mMainPassCB.ViewProjTex, XMMatrixTranspose(viewProjTex));
	XMStoreFloat4x4(&mMainPassCB.ShadowTransform, XMMatrixTranspose(shadowTransform));
	mMainPassCB.EyePosW = m_PrespectiveCamera.GetPosition();
	mMainPassCB.RenderTargetSize = XMFLOAT2((float)m_InitParam.WindowWidth, (float)m_InitParam.WindowHeight);
	mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / m_InitParam.WindowWidth, 1.0f / m_InitParam.WindowHeight);
	mMainPassCB.NearZ = 1.0f;
	mMainPassCB.FarZ = 1000.0f;
	mMainPassCB.TotalTime = time.TotalTime();
	mMainPassCB.DeltaTime = time.DeltaTime();
	mMainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	mMainPassCB.Lights[0].Direction = mRotatedLightDirections[0];
	mMainPassCB.Lights[0].Strength = { 0.9f, 0.9f, 0.7f };
	mMainPassCB.Lights[1].Direction = mRotatedLightDirections[1];
	mMainPassCB.Lights[1].Strength = { 0.4f, 0.4f, 0.4f };
	mMainPassCB.Lights[2].Direction = mRotatedLightDirections[2];
	mMainPassCB.Lights[2].Strength = { 0.2f, 0.2f, 0.2f };

	auto currPassCB = mCurrFrameResource->m_PassUniform.get();
	currPassCB->CopyData(0, mMainPassCB);
}

void DX12RHI::UpdateShadowPass(const RiverTime& time)
{
	XMMATRIX view = XMLoadFloat4x4(&mLightView);
	XMMATRIX proj = XMLoadFloat4x4(&mLightProj);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	UINT w = mShadowMap->Width();
	UINT h = mShadowMap->Height();

	XMStoreFloat4x4(&mShadowPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mShadowPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mShadowPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mShadowPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mShadowPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mShadowPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	mShadowPassCB.EyePosW = mLightPosW;
	mShadowPassCB.RenderTargetSize = XMFLOAT2((float)w, (float)h);
	mShadowPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / w, 1.0f / h);
	mShadowPassCB.NearZ = mLightNearZ;
	mShadowPassCB.FarZ = mLightFarZ;

	auto currPassCB = mCurrFrameResource->m_PassUniform.get();
	currPassCB->CopyData(1, mShadowPassCB);
}

void DX12RHI::UpdateSsaoCBs(const RiverTime& time)
{
	SsaoUniform ssaoCB;

	XMMATRIX P = m_PrespectiveCamera.GetProj();

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	ssaoCB.Proj = mMainPassCB.Proj;
	ssaoCB.InvProj = mMainPassCB.InvProj;
	XMStoreFloat4x4(&ssaoCB.ProjTex, XMMatrixTranspose(P * T));

	mSsao->GetOffsetVectors(ssaoCB.OffsetVectors);

	auto blurWeights = mSsao->CalcGaussWeights(2.5f);
	ssaoCB.BlurWeights[0] = XMFLOAT4(&blurWeights[0]);
	ssaoCB.BlurWeights[1] = XMFLOAT4(&blurWeights[4]);
	ssaoCB.BlurWeights[2] = XMFLOAT4(&blurWeights[8]);

	ssaoCB.InvRenderTargetSize = XMFLOAT2(1.0f / mSsao->SsaoMapWidth(), 1.0f / mSsao->SsaoMapHeight());

	// Coordinates given in view space.
	ssaoCB.OcclusionRadius = 0.5f;
	ssaoCB.OcclusionFadeStart = 0.2f;
	ssaoCB.OcclusionFadeEnd = 2.0f;
	ssaoCB.SurfaceEpsilon = 0.05f;

	auto currSsaoCB = mCurrFrameResource->m_SsaoUniform.get();
	currSsaoCB->CopyData(0, ssaoCB);
}

void DX12RHI::FlushCommandQueue()
{
	mCurrentFence++;

	// Add an instruction to the command queue to set a new fence point.  Because we
	// are on the GPU timeline, the new fence point won't be set until the GPU finishes
	// processing all the commands prior to this Signal().
	ThrowIfFailed(mCommandQueue->Signal(mFence.Get(), mCurrentFence));

	// Wait until the GPU has completed commands up to this fence point.
	if (mFence->GetCompletedValue() < mCurrentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

		// Fire event when GPU hits current fence.
		ThrowIfFailed(mFence->SetEventOnCompletion(mCurrentFence, eventHandle));

		// Wait until the GPU hits current fence event is fired.
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12RHI::CurrentBackBufferView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		mRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		mCurrBackBuffer,
		mRtvDescriptorSize);
}

void DX12RHI::InitDescriptorHeaps()
{
	//
	// Create the SRV heap.
	//
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 64;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap)));

	//
	// Fill out the heap with actual descriptors.
	//
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	std::vector<ComPtr<ID3D12Resource>> tex2DList =
	{
		mTextures["bricksDiffuseMap"]->GetResource(),
		mTextures["bricksNormalMap"]->GetResource(),
		mTextures["tileDiffuseMap"]->GetResource(),
		mTextures["tileNormalMap"]->GetResource(),
		mTextures["defaultDiffuseMap"]->GetResource(),
		mTextures["defaultNormalMap"]->GetResource()
	};

	mSkinnedSrvHeapStart = (UINT)tex2DList.size();

	for (UINT i = 0; i < (UINT)mSkinnedTextureNames.size(); ++i)
	{
		auto texResource = mTextures[mSkinnedTextureNames[i]]->GetResource();
		assert(texResource != nullptr);
		tex2DList.push_back(texResource);
	}


	auto skyCubeMap = mTextures["skyCubeMap"]->GetResource();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	for (UINT i = 0; i < (UINT)tex2DList.size(); ++i)
	{
		srvDesc.Format = tex2DList[i]->GetDesc().Format;
		srvDesc.Texture2D.MipLevels = tex2DList[i]->GetDesc().MipLevels;
		md3dDevice->CreateShaderResourceView(tex2DList[i].Get(), &srvDesc, hDescriptor);

		// next descriptor
		hDescriptor.Offset(1, mCbvSrvUavDescriptorSize);
	}

	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = skyCubeMap->GetDesc().MipLevels;
	srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
	srvDesc.Format = skyCubeMap->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(skyCubeMap.Get(), &srvDesc, hDescriptor);

	mSkyTexHeapIndex = (UINT)tex2DList.size();
	mShadowMapHeapIndex = mSkyTexHeapIndex + 1;
	mSsaoHeapIndexStart = mShadowMapHeapIndex + 1;
	mSsaoAmbientMapIndex = mSsaoHeapIndexStart + 3;
	mNullCubeSrvIndex = mSsaoHeapIndexStart + 5;
	mNullTexSrvIndex1 = mNullCubeSrvIndex + 1;
	mNullTexSrvIndex2 = mNullTexSrvIndex1 + 1;

	auto nullSrv = GetCpuSrv(mNullCubeSrvIndex);
	mNullSrv = GetGpuSrv(mNullCubeSrvIndex);

	md3dDevice->CreateShaderResourceView(nullptr, &srvDesc, nullSrv);
	nullSrv.Offset(1, mCbvSrvUavDescriptorSize);

	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	md3dDevice->CreateShaderResourceView(nullptr, &srvDesc, nullSrv);

	nullSrv.Offset(1, mCbvSrvUavDescriptorSize);
	md3dDevice->CreateShaderResourceView(nullptr, &srvDesc, nullSrv);

	mShadowMap->BuildDescriptors(
		GetCpuSrv(mShadowMapHeapIndex),
		GetGpuSrv(mShadowMapHeapIndex),
		GetDsv(1));

	mSsao->BuildDescriptors(
		mDepthStencilBuffer.Get(),
		GetCpuSrv(mSsaoHeapIndexStart),
		GetGpuSrv(mSsaoHeapIndexStart),
		GetRtv(SwapChainBufferCount),
		mCbvSrvUavDescriptorSize,
		mRtvDescriptorSize);
}

void DX12RHI::InitBaseRenderItems()
{
	auto skyRitem = std::make_unique<DX12RenderItem>();
	XMStoreFloat4x4(&skyRitem->World, XMMatrixScaling(5000.0f, 5000.0f, 5000.0f));
	skyRitem->TexTransform = Identity4x4();
	skyRitem->ObjCBIndex = 0;
	skyRitem->Mat = mMaterials["sky"].get();
	skyRitem->Geo = mGeometries["shapeGeo"].get();
	skyRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	skyRitem->IndexCount = skyRitem->Geo->DrawArgs["sphere"].IndexCount;
	skyRitem->StartIndexLocation = skyRitem->Geo->DrawArgs["sphere"].StartIndexLocation;
	skyRitem->BaseVertexLocation = skyRitem->Geo->DrawArgs["sphere"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Sky].push_back(skyRitem.get());
	mAllRitems.push_back(std::move(skyRitem));

	auto quadRitem = std::make_unique<DX12RenderItem>();
	quadRitem->World = Identity4x4();
	quadRitem->TexTransform = Identity4x4();
	quadRitem->ObjCBIndex = 1;
	quadRitem->Mat = mMaterials["bricks0"].get();
	quadRitem->Geo = mGeometries["shapeGeo"].get();
	quadRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	quadRitem->IndexCount = quadRitem->Geo->DrawArgs["quad"].IndexCount;
	quadRitem->StartIndexLocation = quadRitem->Geo->DrawArgs["quad"].StartIndexLocation;
	quadRitem->BaseVertexLocation = quadRitem->Geo->DrawArgs["quad"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Debug].push_back(quadRitem.get());
	mAllRitems.push_back(std::move(quadRitem));

	auto boxRitem = std::make_unique<DX12RenderItem>();
	XMStoreFloat4x4(&boxRitem->World, XMMatrixScaling(2.0f, 1.0f, 2.0f) * XMMatrixTranslation(0.0f, 0.5f, 0.0f));
	XMStoreFloat4x4(&boxRitem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	boxRitem->ObjCBIndex = 2;
	boxRitem->Mat = mMaterials["bricks0"].get();
	boxRitem->Geo = mGeometries["shapeGeo"].get();
	boxRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	boxRitem->IndexCount = boxRitem->Geo->DrawArgs["box"].IndexCount;
	boxRitem->StartIndexLocation = boxRitem->Geo->DrawArgs["box"].StartIndexLocation;
	boxRitem->BaseVertexLocation = boxRitem->Geo->DrawArgs["box"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(boxRitem.get());
	mAllRitems.push_back(std::move(boxRitem));

	auto gridRitem = std::make_unique<DX12RenderItem>();
	gridRitem->World = Identity4x4();
	XMStoreFloat4x4(&gridRitem->TexTransform, XMMatrixScaling(8.0f, 8.0f, 1.0f));
	gridRitem->ObjCBIndex = 3;
	gridRitem->Mat = mMaterials["tile0"].get();
	gridRitem->Geo = mGeometries["shapeGeo"].get();
	gridRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gridRitem->IndexCount = gridRitem->Geo->DrawArgs["grid"].IndexCount;
	gridRitem->StartIndexLocation = gridRitem->Geo->DrawArgs["grid"].StartIndexLocation;
	gridRitem->BaseVertexLocation = gridRitem->Geo->DrawArgs["grid"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(gridRitem.get());
	mAllRitems.push_back(std::move(gridRitem));

	XMMATRIX brickTexTransform = XMMatrixScaling(1.5f, 2.0f, 1.0f);
	UINT objCBIndex = 4;
	for (int i = 0; i < 5; ++i)
	{
		auto leftCylRitem = std::make_unique<DX12RenderItem>();
		auto rightCylRitem = std::make_unique<DX12RenderItem>();
		auto leftSphereRitem = std::make_unique<DX12RenderItem>();
		auto rightSphereRitem = std::make_unique<DX12RenderItem>();

		XMMATRIX leftCylWorld = XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f);
		XMMATRIX rightCylWorld = XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f);

		XMMATRIX leftSphereWorld = XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f);
		XMMATRIX rightSphereWorld = XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f);

		XMStoreFloat4x4(&leftCylRitem->World, rightCylWorld);
		XMStoreFloat4x4(&leftCylRitem->TexTransform, brickTexTransform);
		leftCylRitem->ObjCBIndex = objCBIndex++;
		leftCylRitem->Mat = mMaterials["bricks0"].get();
		leftCylRitem->Geo = mGeometries["shapeGeo"].get();
		leftCylRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		leftCylRitem->IndexCount = leftCylRitem->Geo->DrawArgs["cylinder"].IndexCount;
		leftCylRitem->StartIndexLocation = leftCylRitem->Geo->DrawArgs["cylinder"].StartIndexLocation;
		leftCylRitem->BaseVertexLocation = leftCylRitem->Geo->DrawArgs["cylinder"].BaseVertexLocation;

		XMStoreFloat4x4(&rightCylRitem->World, leftCylWorld);
		XMStoreFloat4x4(&rightCylRitem->TexTransform, brickTexTransform);
		rightCylRitem->ObjCBIndex = objCBIndex++;
		rightCylRitem->Mat = mMaterials["bricks0"].get();
		rightCylRitem->Geo = mGeometries["shapeGeo"].get();
		rightCylRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		rightCylRitem->IndexCount = rightCylRitem->Geo->DrawArgs["cylinder"].IndexCount;
		rightCylRitem->StartIndexLocation = rightCylRitem->Geo->DrawArgs["cylinder"].StartIndexLocation;
		rightCylRitem->BaseVertexLocation = rightCylRitem->Geo->DrawArgs["cylinder"].BaseVertexLocation;

		XMStoreFloat4x4(&leftSphereRitem->World, leftSphereWorld);
		leftSphereRitem->TexTransform = Identity4x4();
		leftSphereRitem->ObjCBIndex = objCBIndex++;
		leftSphereRitem->Mat = mMaterials["mirror0"].get();
		leftSphereRitem->Geo = mGeometries["shapeGeo"].get();
		leftSphereRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		leftSphereRitem->IndexCount = leftSphereRitem->Geo->DrawArgs["sphere"].IndexCount;
		leftSphereRitem->StartIndexLocation = leftSphereRitem->Geo->DrawArgs["sphere"].StartIndexLocation;
		leftSphereRitem->BaseVertexLocation = leftSphereRitem->Geo->DrawArgs["sphere"].BaseVertexLocation;

		XMStoreFloat4x4(&rightSphereRitem->World, rightSphereWorld);
		rightSphereRitem->TexTransform = Identity4x4();
		rightSphereRitem->ObjCBIndex = objCBIndex++;
		rightSphereRitem->Mat = mMaterials["mirror0"].get();
		rightSphereRitem->Geo = mGeometries["shapeGeo"].get();
		rightSphereRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		rightSphereRitem->IndexCount = rightSphereRitem->Geo->DrawArgs["sphere"].IndexCount;
		rightSphereRitem->StartIndexLocation = rightSphereRitem->Geo->DrawArgs["sphere"].StartIndexLocation;
		rightSphereRitem->BaseVertexLocation = rightSphereRitem->Geo->DrawArgs["sphere"].BaseVertexLocation;

		mRitemLayer[(int)RenderLayer::Opaque].push_back(leftCylRitem.get());
		mRitemLayer[(int)RenderLayer::Opaque].push_back(rightCylRitem.get());
		mRitemLayer[(int)RenderLayer::Opaque].push_back(leftSphereRitem.get());
		mRitemLayer[(int)RenderLayer::Opaque].push_back(rightSphereRitem.get());

		mAllRitems.push_back(std::move(leftCylRitem));
		mAllRitems.push_back(std::move(rightCylRitem));
		mAllRitems.push_back(std::move(leftSphereRitem));
		mAllRitems.push_back(std::move(rightSphereRitem));
	}

	for (UINT i = 0; i < mSkinnedMats.size(); ++i)
	{
		std::string submeshName = "sm_" + std::to_string(i);

		auto ritem = std::make_unique<DX12RenderItem>();

		// Reflect to change coordinate system from the RHS the data was exported out as.
		XMMATRIX modelScale = XMMatrixScaling(0.05f, 0.05f, -0.05f);
		XMMATRIX modelRot = XMMatrixRotationY(PI);
		XMMATRIX modelOffset = XMMatrixTranslation(0.0f, 0.0f, -5.0f);
		XMStoreFloat4x4(&ritem->World, modelScale * modelRot * modelOffset);

		ritem->TexTransform = Identity4x4();
		ritem->ObjCBIndex = objCBIndex++;
		ritem->Mat = mMaterials[mSkinnedMats[i].Name].get();
		ritem->Geo = mGeometries[mSkinnedModelFilename].get();
		ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		ritem->IndexCount = ritem->Geo->DrawArgs[submeshName].IndexCount;
		ritem->StartIndexLocation = ritem->Geo->DrawArgs[submeshName].StartIndexLocation;
		ritem->BaseVertexLocation = ritem->Geo->DrawArgs[submeshName].BaseVertexLocation;

		// All render items for this solider.m3d instance share
		// the same skinned model instance.
		ritem->SkinnedCBIndex = 0;
		ritem->SkinnedModelInst = mSkinnedModelInst.get();

		mRitemLayer[(int)RenderLayer::SkinnedOpaque].push_back(ritem.get());
		mAllRitems.push_back(std::move(ritem));
	}
}

void DX12RHI::InitFrameBuffer()
{
	for (int i = 0; i < s_FrameBufferCount; ++i)
	{
		mFrameBuffer.push_back(MakeUnique<DX12FrameBuffer>(md3dDevice.Get(), 2, (UINT)mAllRitems.size(),
			1, (int)mMaterials.size()));
	}
}

void DX12RHI::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const V_Array<DX12RenderItem*>& ritems)
{
	UINT objCBByteSize = RendererUtil::CalcMinimumGPUAllocSize(sizeof(ObjectUniform));
	UINT skinnedCBByteSize = RendererUtil::CalcMinimumGPUAllocSize(sizeof(SkinnedUniform));

	auto objectCB = mCurrFrameResource->m_ObjectUniform->Resource();
	auto skinnedCB = mCurrFrameResource->m_SkinnedUniform->Resource();

	// For each render item...
	for (size_t i = 0; i < ritems.size(); ++i)
	{
		auto ri = ritems[i];

		cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
		cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
		cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;

		cmdList->SetGraphicsRootConstantBufferView(0, objCBAddress);

		if (ri->SkinnedModelInst != nullptr)
		{
			D3D12_GPU_VIRTUAL_ADDRESS skinnedCBAddress = skinnedCB->GetGPUVirtualAddress() + ri->SkinnedCBIndex * skinnedCBByteSize;
			cmdList->SetGraphicsRootConstantBufferView(1, skinnedCBAddress);
		}
		else
		{
			cmdList->SetGraphicsRootConstantBufferView(1, 0);
		}

		cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
	}
}

void DX12RHI::DrawSceneToShadowMap()
{
	mCommandList->RSSetViewports(1, &mShadowMap->Viewport());
	mCommandList->RSSetScissorRects(1, &mShadowMap->ScissorRect());

	// Change to DEPTH_WRITE.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap->Resource(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	// Clear the back buffer and depth buffer.
	mCommandList->ClearDepthStencilView(mShadowMap->Dsv(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	mCommandList->OMSetRenderTargets(0, nullptr, false, &mShadowMap->Dsv());

	// Bind the pass constant buffer for the shadow map pass.
	UINT passCBByteSize = RendererUtil::CalcMinimumGPUAllocSize(sizeof(PassUniform));
	auto passCB = mCurrFrameResource->m_PassUniform->Resource();
	D3D12_GPU_VIRTUAL_ADDRESS passCBAddress = passCB->GetGPUVirtualAddress() + 1 * passCBByteSize;
	mCommandList->SetGraphicsRootConstantBufferView(2, passCBAddress);

	//mCommandList->SetPipelineState(mPSOs["shadow_opaque"].Get());
	mCommandList->SetPipelineState(m_PSOs["shadow_opaque"]->GetPSO());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Opaque]);

	//mCommandList->SetPipelineState(mPSOs["skinnedShadow_opaque"].Get());
	mCommandList->SetPipelineState(m_PSOs["skinnedShadow_opaque"]->GetPSO());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::SkinnedOpaque]);

	// Change back to GENERIC_READ so we can read the texture in a shader.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap->Resource(),
		D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));
}

void DX12RHI::DrawNormalsAndDepth()
{
	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	auto normalMap = mSsao->NormalMap();
	auto normalMapRtv = mSsao->NormalMapRtv();

	// Change to RENDER_TARGET.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(normalMap,
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Clear the screen normal map and depth buffer.
	float clearValue[] = { 0.0f, 0.0f, 1.0f, 0.0f };
	mCommandList->ClearRenderTargetView(normalMapRtv, clearValue, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	mCommandList->OMSetRenderTargets(1, &normalMapRtv, true, &DepthStencilView());

	// Bind the constant buffer for this pass.
	auto passCB = mCurrFrameResource->m_PassUniform->Resource();
	mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	//mCommandList->SetPipelineState(mPSOs["drawNormals"].Get());
	mCommandList->SetPipelineState(m_PSOs["drawNormals"]->GetPSO());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Opaque]);

	//mCommandList->SetPipelineState(mPSOs["skinnedDrawNormals"].Get());
	mCommandList->SetPipelineState(m_PSOs["skinnedDrawNormals"]->GetPSO());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::SkinnedOpaque]);

	// Change back to GENERIC_READ so we can read the texture in a shader.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(normalMap,
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
}

void DX12RHI::CreateSRV(CD3DX12_CPU_DESCRIPTOR_HANDLE& handle, ID3D12Resource* textureRes, D3D12_SHADER_RESOURCE_VIEW_DESC& desc, uint32_t handleOffset)
{
	if (handleOffset > 0)
	{
		handle.Offset(handleOffset, mCbvSrvUavDescriptorSize);
	}

	return md3dDevice->CreateShaderResourceView(textureRes, &desc, handle);
}
