#include "RiverPch.h"
#include "RendererUtil.h"

#include "Renderer/Font/Header/FontAtlas.h"
#include "Renderer/Font/Header/Font.h"
#include "Renderer/Header/AssetManager.h"
#include "Renderer/DX12Renderer/Header/d3dx12.h"
#include "Renderer/DX12Renderer/Header/DX12RHI.h"
#include "Renderer/DX12Renderer/Header/DX12CommandPool.h"
#include "Renderer/DX12Renderer/Header/DX12DynamicDescriptorHeap.h"
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
#include "Renderer/DX12Renderer/Header/DX12DescriptorAllocator.h"

#include "Renderer/Header/GeometryGenerator.h"
#include "Renderer/Header/Material.h"

#include "DirectXMath.h"
#include "DirectXCollision.h"
#include <d3dcompiler.h>
#include <iostream>
#include <chrono>
#include <fstream>

#if defined(DEBUG) || defined(_DEBUG)
	#define _CRTDBG_MAP_ALLOC
	#include <crtdbg.h>
	#include "pix3.h"
#endif

#include "Renderer/DX12Renderer/Header/DX12DefaultConfig.h"

//Material* TestMaterial;

using Microsoft::WRL::ComPtr;

DirectX::BoundingSphere mSceneBounds;
float mLightNearZ = 0.0f;
float mLightFarZ = 0.0f;
DirectX::XMFLOAT3 mLightPosW;
DirectX::XMFLOAT4X4 mLightView = Identity4x4();
DirectX::XMFLOAT4X4 mLightProj = Identity4x4();
DirectX::XMFLOAT4X4 mShadowTransform = Identity4x4();

DirectX::BoundingFrustum mCamFrustum;
DX12RenderItem* mPickedRitem = nullptr;
UINT mShadowMapHeapIndex;
UINT mSsaoHeapIndexStart = 0;
UINT mSsaoAmbientMapIndex = 0;
UINT mNullCubeSrvIndex = 0;
UINT mNullTexSrvIndex1 = 0;
UINT mNullTexSrvIndex2 = 0;
CD3DX12_GPU_DESCRIPTOR_HANDLE mNullSrv;

float mLightRotationAngle = 0.0f;
DirectX::XMFLOAT3 mBaseLightDirections[3] = {
	DirectX::XMFLOAT3(0.57735f, -0.57735f, 0.57735f),
	DirectX::XMFLOAT3(-0.57735f, -0.57735f, 0.57735f),
	DirectX::XMFLOAT3(0.0f, -0.707f, -0.707f)
};
DirectX::XMFLOAT3 mRotatedLightDirections[3];
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

		SkinnedInfo->GetFinalTransforms(ClipName, TimePos, FinalTransforms);
	}
};

std::unique_ptr<SkinnedModelInstance> mSkinnedModelInst;
SkinnedData mSkinnedInfo;
std::vector<M3DLoader::Subset> mSkinnedSubsets;
std::vector<M3DLoader::M3dMaterial> mSkinnedMats;
std::vector<std::string> mSkinnedTextureNames;

static D3D12_PRIMITIVE_TOPOLOGY GetRenderItemPrimtiveType(PrimitiveType type)
{
	switch (type)
	{
	case PrimitiveType::TriangleList:
		return D3D12_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	default:
		break;
	}

	return D3D12_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}


DX12RHI::DX12RHI()
	: m_CurrentFence(1), m_CurrFrameResourceIndex(0), m_CurrBackBufferIndex(0),
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
#ifdef _DEBUG
	PIXLoadLatestWinPixGpuCapturerLibrary();
#endif // _DEBUG

	m_InitParam = param;

	/*m_PrespectiveCamera.LookAt(DirectX::XMFLOAT3(5.0f, 4.0f, -15.0f),
		DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),
		DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));*/
	mSceneBounds.Center = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	mSceneBounds.Radius = sqrtf(10.0f * 10.0f + 15.0f * 15.0f);

	InitializeBase(param);

	ThrowIfFailed(m_CommandList->Reset(m_CommandAllocator.Get(), nullptr));

	m_PrespectiveCamera.SetPosition(0.0f, 2.0f, -15.0f);

	{
		m_ShadowMap = std::make_unique<ShadowMap>(m_Device.Get(), 2048, 2048);
		m_Ssao = std::make_unique<Ssao>(m_Device.Get(), m_CommandList.Get(), param.WindowWidth, param.WindowHeight);
		m_SrvDescriptorHeap = DX12DescriptorAllocator::GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		InitBaseGeometry();
		InitBaseTexture();

		/*m_Fonts["default"] = MakeUnique<FontAtlas>(DEFAULT_FONT_PATH_1, 16.0f);
		CreateTexture("font", m_Fonts["default"]->GetTextureWidth(), m_Fonts["default"]->GetTextureHeight(), m_Fonts["default"]->GetTextureDataRGBA32());

		CreateTexture("bricksDiffuseMap", DEFAULT_TEXTURE_PATH_13);
		CreateTexture("bricksNormalMap", DEFAULT_TEXTURE_PATH_14);
		CreateTexture("tileDiffuseMap", DEFAULT_TEXTURE_PATH_3);
		CreateTexture("tileNormalMap", DEFAULT_TEXTURE_PATH_15);
		CreateTexture("skyCubeMap", DEFAULT_TEXTURE_PATH_18);*/

		//LoadSkinnedModel();
		//LoadTextures();
		//InitBaseRootSignatures();
		InitBaseShaders();

		m_RawMeshVertexBuffer = CreateUploadVertexBuffer((float*)V_Array<RawVertex>(1000).data(), 1000 * sizeof(RawVertex), (uint32)sizeof(RawVertex), &m_InputLayers["defaultRaw"]);
		m_RawMeshIndexBuffer = CreateUploadIndexBuffer(V_Array<uint16>(3000).data(), 3000, ShaderDataType::Short);

		m_UIVertexBuffer = CreateUploadVertexBuffer((float*)V_Array<UIVertex>(1000).data(), 1000 * sizeof(UIVertex), (uint32)sizeof(UIVertex), &m_InputLayers["ui"]);
		m_UIIndexBuffer = CreateUploadIndexBuffer(V_Array<uint16>(3000).data(), 3000, ShaderDataType::Short);

		InitBaseMaterials();
		InitFrameBuffer();
		//InitBasePSOs();

		//m_Ssao->SetPSOs(m_PSOs["ssao"]->GetPSO(), m_PSOs["ssaoBlur"]->GetPSO());
	}

	ThrowIfFailed(m_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	FlushCommandQueue();
}

void DX12RHI::Exit()
{
	DX12DescriptorAllocator::DestroyAllDescriptorHeap();
}

void DX12RHI::BeginFrame()
{
	WaitFence();

	//添加与删除Uniform数据
	auto& currObjectCB = m_CurrFrameResource->m_ObjectUniform;
	for (auto& renderItem : m_RenderItemAllocator.m_Containor)
	{
		if (renderItem.ObjCBIndex != (uint32)-1 && renderItem.NumFramesDirty > 0)
		{
			renderItem.NumFramesDirty--;

			DirectX::XMMATRIX world = XMLoadFloat4x4((const XMFLOAT4X4*)(&renderItem.World));
			DirectX::XMMATRIX texTransform = XMLoadFloat4x4((const XMFLOAT4X4*)(&renderItem.TexTransform));

			ObjectUniform objConstants;
			XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(world));
			XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
			objConstants.MaterialIndex = 2;

			currObjectCB->CopyData(renderItem.ObjCBIndex, objConstants);

		}
	}

	//重置动态描述偏移
	m_DynamicDescriptorOffset[m_CurrFrameResourceIndex] = 0;
}

void DX12RHI::EndFrame()
{
	ThrowIfFailed(m_SwapChain->Present(0, 0));
	m_CurrBackBufferIndex = (m_CurrBackBufferIndex + 1) % s_SwapChainBufferCount;

	m_CurrFrameResource->m_FenceValue = ++m_CurrentFence;

	m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence);
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

	m_CurrFrameResourceIndex = (m_CurrFrameResourceIndex + 1) % RHI::GetFrameCount();
	m_CurrFrameResource = m_FrameBuffer[m_CurrFrameResourceIndex].get();
	WaitFence();

	UpdateObjectCBs();
	//UpdateSkinnedCBs(time);
	//UpdateMaterialCBs();
	//UpdateShadowTransform(time);
	UpdateMainPass();
	//UpdateShadowPass(time);
	//UpdateSsaoCBs(time);
}

void DX12RHI::UpdateSceneData(const V_Array<RawVertex>& vertices, const V_Array<uint16_t> indices)
{
	{
		if (!m_CurrFrameResource)
		{
			return;
		}

		int index = 0;
		auto& currObjectCB = m_CurrFrameResource->m_ObjectUniform;
		for (auto& it : m_RenderItemAllocator.m_Containor)
		{

			/*m_RenderItems[i].VertexBufferView = TestVertexBuffer.get();
			m_RenderItems[i].IndexBufferView = TestIndexBuffer.get();*/
			DirectX::XMMATRIX world = XMLoadFloat4x4((const XMFLOAT4X4*)(&it.World));
			DirectX::XMMATRIX texTransform = XMLoadFloat4x4((const XMFLOAT4X4*)(&it.TexTransform));

			ObjectUniform objConstants;
			XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(world));
			XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
			objConstants.MaterialIndex = 2;
			it.ObjCBIndex = index++;

			currObjectCB->CopyData(it.ObjCBIndex, objConstants);

			if (!it.VertexBuffer)
			{
				it.VertexBuffer = m_RawMeshVertexBuffer.get();
			}

			if (!it.IndexBuffer)
			{
				it.IndexBuffer = m_RawMeshIndexBuffer.get();
			}
		}
	}

	if (vertices.size() > 0 && indices.size() > 0)
	{
		m_RawMeshVertexBuffer->UpdateData(m_Device.Get(), m_CommandList.Get(), (void*)vertices.data(), (uint32)vertices.size(), 1000);
		m_RawMeshIndexBuffer->UpdateData(m_Device.Get(), m_CommandList.Get(), (void*)indices.data(), (uint32)indices.size(), 3000);
	}

	//auto& geo = m_Geometries["scene"];
	//UINT verticesMore = (UINT)vertices.size() + 1000;
	//UINT indicesMore = (UINT)indices.size() + 3000;
	//if (geo == nullptr)
	//{
	//	const UINT vbByteSize = verticesMore * sizeof(RawVertex);
	//	const UINT ibByteSize = indicesMore * sizeof(std::uint16_t);

	//	auto geo = MakeUnique<MeshGeometry>();
	//	geo->m_Name = "shapeGeo";
	//	//geo->CopyCPUData(vertices, indices);
	//	geo->SetVertexBufferAndIndexBuffer(CreateUploadVertexBuffer((float*)vertices.data(), vbByteSize, (UINT)sizeof(RawVertex), &m_InputLayers["defaultRaw"]),
	//		CreateUploadIndexBuffer((void*)indices.data(), (UINT)indices.size(), ShaderDataType::Short));

	//	m_Geometries["scene"] = std::move(geo);
	//}
	//else
	//{
	//	geo->m_VertexBuffer->UpdateData(m_Device.Get(), m_CommandList.Get(), (void*)vertices.data(), (uint32)vertices.size(), 1000);
	//	geo->m_IndexBuffer->UpdateData(m_Device.Get(), m_CommandList.Get(), (void*)indices.data(), (uint32)indices.size(), 3000);
	//}

}

void DX12RHI::UpdateUIData(V_Array<UIVertex>& vertices, V_Array<uint16> indices)
{
	auto& currObjectCB = m_CurrFrameResource->m_ObjectUniform;
	
	int index = (int)m_RenderItemAllocator.m_Containor.size();
	for (size_t i = 0; i < m_UIRenderItemAllocator.m_Containor.size(); i++)
	{
		m_UIRenderItemAllocator.m_Containor[i].ObjCBIndex = index++;
		DirectX::XMMATRIX world = XMLoadFloat4x4((const XMFLOAT4X4*)(&m_UIRenderItemAllocator.m_Containor[i].World));

		ObjectUniform objConstants;
		XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(world));

		currObjectCB->CopyData(m_UIRenderItemAllocator.m_Containor[i].ObjCBIndex, objConstants);
	}

	m_UIVertexBuffer->UpdateData(m_Device.Get(), m_CommandList.Get(), vertices.data(), (uint32)vertices.size(), 1000);
	m_UIIndexBuffer->UpdateData(m_Device.Get(), m_CommandList.Get(), indices.data(), (uint32)indices.size(), 3000);

	m_UIRenderItem.IndexCount = (int)indices.size();
}

void DX12RHI::SetUpStaticMesh(StaticMesh* mesh)
{
	const uint64 vbByteSize = mesh->GetVertices().size() * sizeof(Vertex);
	const uint64 ibByteSize = mesh->GetIndices().size() * sizeof(uint32);

	if (m_MeshBuffer.find(mesh->GetName()) == m_MeshBuffer.end())
	{
		m_MeshBuffer[mesh->GetName()] = {
			CreateVertexBuffer((float*)mesh->GetVertices().data(), (uint32)vbByteSize, (uint32)sizeof(DX12Vertex), &m_InputLayers["default"]),
			CreateIndexBuffer((void*)mesh->GetIndices().data(), (uint32)mesh->GetIndices().size(), ShaderDataType::Int)
		};
	}
}

Pair<void*, void*> DX12RHI::GetStaticMeshBuffer(const char* name)
{
	auto iter = m_MeshBuffer.find(name);
	if (iter != m_MeshBuffer.end())
	{
		return { iter->second.first.get(), iter->second.second.get() };
	}

	return { nullptr, nullptr };
}

Material* DX12RHI::CreateMaterial(const char* name)
{
	auto iter = m_Materials.find(name);
	if (iter != m_Materials.end())
	{
		return iter->second.get();
	}

	m_Materials[name] = MakeUnique<Material>(name);
	return m_Materials[name].get();
}

void DX12RHI::SetViewPort(uint32 w, uint32 h, uint32 xOffset, uint32 yOffset)
{
	m_ScreenViewport.Width = (float)w;
	m_ScreenViewport.Height = (float)h;
	m_ScreenViewport.TopLeftX = (float)xOffset;
	m_ScreenViewport.TopLeftY = (float)yOffset;
}

Unique<Texture> DX12RHI::CreateTexture(const char* name, const char* path, bool isImmediately)
{
	if (isImmediately)
	{
		WaitFence();
		ResetCmdListAlloc();
	}

	auto texture = MakeUnique<DX12Texture>(m_Device.Get(), m_CommandList.Get(), name, path);
	
	if (isImmediately)
	{
		AddDescriptor(texture.get());
		ExecuteCmdList(false);
		WaitFence();
	}
	

	return texture;
}

Unique<Shader> DX12RHI::CreateShader(const char* name, const char* path)
{
	auto layout = &m_InputLayers["default"];
	if (name == String("ui"))
	{
		layout = &m_InputLayers["ui"];
	}
	return MakeUnique<DX12Shader>(m_Device.Get(), path, Pair<const D3D_SHADER_MACRO*, const D3D_SHADER_MACRO*>(nullptr, nullptr),
		Pair<const char*, const char*>("VS", "PS"), Pair<const char*, const char*>{ "vs_5_1", "ps_5_1" }, layout);
}

void DX12RHI::GenerateDrawCommands(int commandId, FrameBufferType frameBufferType)
{
	auto assetManager = AssetManager::Get();
	auto& cmdListAlloc = DX12CommandPool::GetCommandAllocator(commandId)[m_CurrFrameResourceIndex];
	//auto cmdListAlloc = m_CurrFrameResource->m_CommandAlloc;
	ThrowIfFailed(cmdListAlloc->Reset());
	//auto commandList = m_CommandList;
	auto& commandList = DX12CommandPool::GetCommandList(commandId)[m_CurrFrameResourceIndex];
	ThrowIfFailed(commandList->Reset(cmdListAlloc.Get(), nullptr));


	//m_CommandList->SetGraphicsRootSignature(m_RootSignatures["default"]->GetRootSignature());

	//绑定材质缓冲数据
	/*auto matBuffer = m_CurrFrameResource->m_MaterialUniform->Resource();
	m_CommandList->SetGraphicsRootShaderResourceView(3, matBuffer->GetGPUVirtualAddress());*/

	commandList->RSSetViewports(1, &m_ScreenViewport);
	commandList->RSSetScissorRects(1, &m_ScissorRect);

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	auto& dynamicHeaps = m_DynamicDescriptorHeaps[m_CurrFrameResourceIndex];
	UINT objCBByteSize = RendererUtil::CalcMinimumGPUAllocSize(sizeof(ObjectUniform));
	auto objectCB = m_CurrFrameResource->m_ObjectUniform->Resource();
	auto passCB = m_CurrFrameResource->m_PassUniform->Resource();

	OnSetRenderTargets(commandId, frameBufferType);

	int offset = 0;
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_DynamicDescriptorHeaps[m_CurrFrameResourceIndex]->GetHeap() };
	commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	CD3DX12_CPU_DESCRIPTOR_HANDLE dynamicHandle(dynamicHeaps->GetCpuHeapStart());
	dynamicHandle.Offset(m_DynamicDescriptorOffset[m_CurrFrameResourceIndex], DescriptorUtils::GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	CD3DX12_GPU_DESCRIPTOR_HANDLE texDescriptor(dynamicHeaps->GetGpuHeapStart());

	if (frameBufferType == FrameBufferType::UI)
	{
		for (size_t i = 0; i < m_UIRenderItemAllocator.m_Containor.size(); i++)
		{
			auto& renderItem = m_UIRenderItemAllocator.m_Containor[i];
			texDescriptor.Offset(m_DynamicDescriptorOffset[m_CurrFrameResourceIndex] - offset, DescriptorUtils::GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
			offset = m_DynamicDescriptorOffset[m_CurrFrameResourceIndex];

			bool setPipeline = false;
			if (renderItem.Material)
			{
				auto mat = renderItem.Material;
				if (mat->m_DiffuseTexture)
				{
					m_Device->CopyDescriptorsSimple(1, dynamicHandle, { mat->m_DiffuseTexture->GetTextureHandle() },
						D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

					dynamicHandle.Offset(1, DescriptorUtils::GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
					m_DynamicDescriptorOffset[m_CurrFrameResourceIndex]++;
				}
				if (mat->m_NormalTexture)
				{
					m_Device->CopyDescriptorsSimple(1, dynamicHandle, { mat->m_NormalTexture->GetTextureHandle() },
						D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

					dynamicHandle.Offset(1, DescriptorUtils::GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
					m_DynamicDescriptorOffset[m_CurrFrameResourceIndex]++;
				}

				auto shader = static_cast<DX12Shader*>(mat->m_Shader);
				if (shader)
				{
					commandList->SetGraphicsRootSignature(shader->GetRootSignaure());
					commandList->SetPipelineState(shader->GetPipelineState());
					setPipeline = true;
				}
			}
			
			if (!setPipeline)
			{
				auto shader = static_cast<DX12Shader*>(assetManager->GetShader("ui"));
				commandList->SetGraphicsRootSignature(shader->GetRootSignaure());
				commandList->SetPipelineState(shader->GetPipelineState());
			}

			commandList->SetGraphicsRootDescriptorTable(5, texDescriptor);
			commandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());


			if (renderItem.RenderTexture)
			{
				m_Device->CopyDescriptorsSimple(1, dynamicHandle, 
					{ renderItem.RenderTexture->GetTextureHandle() }, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

				dynamicHandle.Offset(1, DescriptorUtils::GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
				m_DynamicDescriptorOffset[m_CurrFrameResourceIndex]++;
			}

			commandList->IASetVertexBuffers(0, 1, &m_UIVertexBuffer->GetView());
			commandList->IASetIndexBuffer(&m_UIIndexBuffer->GetView());
			commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + renderItem.ObjCBIndex * objCBByteSize;

			commandList->SetGraphicsRootConstantBufferView(0, objCBAddress);
			commandList->SetGraphicsRootConstantBufferView(1, 0);
			commandList->DrawIndexedInstanced(renderItem.IndexCount, 1,
				renderItem.StartIndexLocation, renderItem.BaseVertexLocation, 0);
		}
	}
	else
	{
		
		for (auto id : /*m_RenderItems*/m_DrawItems)
		{
			auto& it = m_RenderItemAllocator.m_Containor[id];
			
			texDescriptor.Offset(m_DynamicDescriptorOffset[m_CurrFrameResourceIndex] - offset, DescriptorUtils::GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
			offset = m_DynamicDescriptorOffset[m_CurrFrameResourceIndex];
			/*if (it.first == "sky")
			{
				CD3DX12_GPU_DESCRIPTOR_HANDLE texDescriptor(dynamicHeaps->GetGpuHeapStart());
				texDescriptor.Offset(offset, DescriptorUtils::GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
				m_CommandList->SetGraphicsRootDescriptorTable(4, texDescriptor);
			}*/

			bool setPipeline = false;
			if (it.Material)
			{
				auto mat = it.Material;
				if (mat->m_DiffuseTexture)
				{
					m_Device->CopyDescriptorsSimple(1, dynamicHandle, { mat->m_DiffuseTexture->GetTextureHandle() },
						D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

					dynamicHandle.Offset(1, DescriptorUtils::GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
					m_DynamicDescriptorOffset[m_CurrFrameResourceIndex]++;
				}
				if (mat->m_NormalTexture)
				{
					m_Device->CopyDescriptorsSimple(1, dynamicHandle, { mat->m_NormalTexture->GetTextureHandle() },
						D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

					dynamicHandle.Offset(1, DescriptorUtils::GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
					m_DynamicDescriptorOffset[m_CurrFrameResourceIndex]++;
				}

				auto shader = static_cast<DX12Shader*>(mat->m_Shader);
				if (shader)
				{
					commandList->SetGraphicsRootSignature(shader->GetRootSignaure());
					commandList->SetPipelineState(shader->GetPipelineState());
					setPipeline = true;
				}
			}

			if (!setPipeline)
			{
				auto shader = static_cast<DX12Shader*>(assetManager->GetShader("default"));
				commandList->SetGraphicsRootSignature(shader->GetRootSignaure());
				commandList->SetPipelineState(shader->GetPipelineState());
			}

			commandList->SetGraphicsRootDescriptorTable(5, texDescriptor);
			commandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

			commandList->IASetVertexBuffers(0, 1, &((DX12VertexBuffer*)it.VertexBuffer)->GetView());
			commandList->IASetIndexBuffer(&((DX12IndexBuffer*)it.IndexBuffer)->GetView());
			commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + it.ObjCBIndex * objCBByteSize;

			commandList->SetGraphicsRootConstantBufferView(0, objCBAddress);
			commandList->SetGraphicsRootConstantBufferView(1, 0);
			commandList->DrawIndexedInstanced(it.IndexCount, 1,
				it.StartIndexLocation, it.BaseVertexLocation, 0);
		}
	}


	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// Done recording commands.
	ThrowIfFailed(commandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { commandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	m_DrawItems.clear();
}

void DX12RHI::OnSetRenderTargets(int commandId, FrameBufferType frameBufferType)
{
	auto& commandList = DX12CommandPool::GetCommandList(commandId)[m_CurrFrameResourceIndex];
	switch (frameBufferType)
	{
	case FrameBufferType::Color:
	{
		commandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
		commandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		commandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());
	}
		break;
	case FrameBufferType::UI:
	{
		commandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		commandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());
	}
		break;
	default:
		break;
	}
}

int DX12RHI::AllocDrawCommand()
{
	return DX12CommandPool::AllocaCommand();
}

void DX12RHI::DrawRenderItem(int renderItemId)
{
	m_DrawItems.push_back(renderItemId);
}

void DX12RHI::AddDescriptor(DX12Texture* texture)
{
	auto nullSrv = DX12DescriptorAllocator::CpuOffset(m_SrvDescriptorHeap, mNullTexSrvIndex2);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	srvDesc.Format = texture->GetResource()->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = texture->GetResource()->GetDesc().MipLevels;
	m_Device->CreateShaderResourceView(texture->GetResource().Get(), &srvDesc, nullSrv);
}

Unique<Texture> DX12RHI::CreateTexture(const char* name, int width, int height, const uint8* data)
{
	return MakeUnique<DX12Texture>(m_Device.Get(), m_CommandList.Get(), name, data, width, height);;
}

Unique<Texture> DX12RHI::CreateTextureWithResource(const char* name, void* resource)
{
	return MakeUnique<DX12Texture>(name, static_cast<Microsoft::WRL::ComPtr<ID3D12Resource>*>(resource));
}

void DX12RHI::Render()
{
	auto cmdListAlloc = m_CurrFrameResource->m_CommandAlloc;

	//重复使用与命令录制相关的内存。只有当关联的命令列表在GPU上完成执行时，我们才能重置
	ThrowIfFailed(cmdListAlloc->Reset());

	// 命令列表可以在通过ExecuteCommandList添加到命令队列后重置，重复使用命令列表会重复使用内存。
	ThrowIfFailed(m_CommandList->Reset(cmdListAlloc.Get(), m_PSOs["opaque"]->GetPSO()));

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_SrvDescriptorHeap };
	m_CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	m_CommandList->SetGraphicsRootSignature(m_RootSignatures["default"]->GetRootSignature());

	//
	// Shadow map pass.
	//

	// Bind all the materials used in this scene.  For structured buffers, we can bypass the heap and 
	// set as a root descriptor.
	auto matBuffer = m_CurrFrameResource->m_MaterialUniform->Resource();
	m_CommandList->SetGraphicsRootShaderResourceView(3, matBuffer->GetGPUVirtualAddress());

	// Bind null SRV for shadow map pass.
	//m_CommandList->SetGraphicsRootDescriptorTable(4, mNullSrv);

	//绑定纹理贴图，只需要提供起始位置，根签名能自动知道所需数据大小
	auto handle = m_SrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_CommandList->SetGraphicsRootDescriptorTable(5, handle);

	//DrawSceneToShadowMap();

	//DrawNormalsAndDepth();

	//m_CommandList->SetGraphicsRootSignature(m_RootSignatures["ssao"]->GetRootSignature());
	//m_Ssao->ComputeSsao(m_CommandList.Get(), m_CurrFrameResource, 2);

	m_CommandList->SetGraphicsRootSignature(m_RootSignatures["default"]->GetRootSignature());

	//绑定材质缓冲数据
	matBuffer = m_CurrFrameResource->m_MaterialUniform->Resource();
	m_CommandList->SetGraphicsRootShaderResourceView(3, matBuffer->GetGPUVirtualAddress());

	m_CommandList->RSSetViewports(1, &m_ScreenViewport);
	m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	//清除后台缓冲与深度缓冲
	m_CommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	m_CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	m_CommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	m_CommandList->SetGraphicsRootDescriptorTable(5, m_SrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	auto passCB = m_CurrFrameResource->m_PassUniform->Resource();
	m_CommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	// Bind the sky cube map.  For our demos, we just use one "world" cube map representing the environment
	// from far away, so all objects will use the same cube map and we only need to set it once per-frame.  
	// If we wanted to use "local" cube maps, we would have to change them per-object, or dynamically
	// index into an array of cube maps.

	/*CD3DX12_GPU_DESCRIPTOR_HANDLE skyTexDescriptor(m_SrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	skyTexDescriptor.Offset(m_Textures["skyCubeMap"]->GetTextureId(), DescriptorUtils::GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	m_CommandList->SetGraphicsRootDescriptorTable(4, skyTexDescriptor);*/

	m_CommandList->SetPipelineState(m_PSOs["opaque"]->GetPSO());
	//DrawRenderItems(m_CommandList.Get(), m_RitemLayer[(int)RenderLayer::Opaque]);

	{
		//m_CommandList->SetPipelineState(m_PSOs["opaqueRaw"]->GetPSO());
		//auto& geo = m_Geometries["scene"];
		//if (geo)
		{
			ID3D12DescriptorHeap* descriptorHeaps[] = { m_DynamicDescriptorHeaps[m_CurrFrameResourceIndex]->GetHeap() };
			m_CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

			auto& dynamicHeaps = m_DynamicDescriptorHeaps[m_CurrFrameResourceIndex];
			UINT objCBByteSize = RendererUtil::CalcMinimumGPUAllocSize(sizeof(ObjectUniform));
			auto objectCB = m_CurrFrameResource->m_ObjectUniform->Resource();


			uint32 offset = 0;
			CD3DX12_CPU_DESCRIPTOR_HANDLE dynamicHandle(dynamicHeaps->GetCpuHeapStart());
			CD3DX12_GPU_DESCRIPTOR_HANDLE texDescriptor(dynamicHeaps->GetGpuHeapStart());
			for (auto it : m_RenderItemAllocator.m_Containor)
			{
				m_CommandList->SetPipelineState(m_PSOs["opaque"]->GetPSO());


				texDescriptor.Offset(offset, DescriptorUtils::GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
				m_CommandList->SetGraphicsRootDescriptorTable(5, texDescriptor);

				/*if (it.first == "sky")
				{
					CD3DX12_GPU_DESCRIPTOR_HANDLE texDescriptor(dynamicHeaps->GetGpuHeapStart());
					texDescriptor.Offset(offset, DescriptorUtils::GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
					m_CommandList->SetGraphicsRootDescriptorTable(4, texDescriptor);
				}*/

				if (it.Material)
				{
					auto mat = it.Material;
					if (mat->m_DiffuseTexture)
					{
						m_Device->CopyDescriptorsSimple(1, dynamicHandle, { mat->m_DiffuseTexture->GetTextureHandle() },
							D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

						dynamicHandle.Offset(1, DescriptorUtils::GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
						offset++;
					}
					if (mat->m_NormalTexture)
					{
						m_Device->CopyDescriptorsSimple(1, dynamicHandle, { mat->m_NormalTexture->GetTextureHandle() },
							D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

						dynamicHandle.Offset(1, DescriptorUtils::GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
						offset++;
					}
				}

				m_CommandList->IASetVertexBuffers(0, 1, &((DX12VertexBuffer*)it.VertexBuffer)->GetView());
				m_CommandList->IASetIndexBuffer(&((DX12IndexBuffer*)it.IndexBuffer)->GetView());
				m_CommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + /*it.ObjCBIndex*/0 * objCBByteSize;

				m_CommandList->SetGraphicsRootConstantBufferView(0, objCBAddress);
				m_CommandList->SetGraphicsRootConstantBufferView(1, 0);
				m_CommandList->DrawIndexedInstanced(it.IndexCount, 1,
					it.StartIndexLocation, it.BaseVertexLocation, 0);
			}
		}
	}

	{
		m_CommandList->SetPipelineState(m_PSOs["ui"]->GetPSO());
		//auto& geo = m_Geometries["ui"];
		//if (geo)
		{
			CD3DX12_GPU_DESCRIPTOR_HANDLE texDescriptor(m_SrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
			//texDescriptor.Offset(0, DescriptorUtils::GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
			m_CommandList->SetGraphicsRootDescriptorTable(5, texDescriptor);
			UINT objCBByteSize = RendererUtil::CalcMinimumGPUAllocSize(sizeof(ObjectUniform));
			auto objectCB = m_CurrFrameResource->m_ObjectUniform->Resource();

			for (size_t i = 0; i < m_UIRenderItemAllocator.m_Containor.size(); i++)
			{
				m_CommandList->IASetVertexBuffers(0, 1, &m_UIVertexBuffer->GetView());
				m_CommandList->IASetIndexBuffer(&m_UIIndexBuffer->GetView());
				m_CommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + m_UIRenderItemAllocator.m_Containor[i].ObjCBIndex * objCBByteSize;

				m_CommandList->SetGraphicsRootConstantBufferView(0, objCBAddress);
				m_CommandList->SetGraphicsRootConstantBufferView(1, 0);
				m_CommandList->DrawIndexedInstanced(m_UIRenderItemAllocator.m_Containor[i].IndexCount, 1,
					m_UIRenderItemAllocator.m_Containor[i].StartIndexLocation, m_UIRenderItemAllocator.m_Containor[i].BaseVertexLocation, 0);
			}

			//m_CommandList->SetGraphicsRootDescriptorTable(5, m_SrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		}
	}

	//mCommandList->SetPipelineState(mPSOs["skinnedOpaque"].Get());
	/*m_CommandList->SetPipelineState(m_PSOs["skinnedOpaque"]->GetPSO());
	DrawRenderItems(m_CommandList.Get(), m_RitemLayer[(int)RenderLayer::SkinnedOpaque]);

	m_CommandList->SetPipelineState(m_PSOs["debug"]->GetPSO());
	DrawRenderItems(m_CommandList.Get(), m_RitemLayer[(int)RenderLayer::Debug]);

	m_CommandList->SetPipelineState(m_PSOs["sky"]->GetPSO());
	DrawRenderItems(m_CommandList.Get(), m_RitemLayer[(int)RenderLayer::Sky]);*/

	/*mCommandList->SetPipelineState(m_PSOs["ui"]->GetPSO());
	DrawUI();*/

	// Indicate a state transition on the resource usage.
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// Done recording commands.
	ThrowIfFailed(m_CommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Swap the back and front buffers
	ThrowIfFailed(m_SwapChain->Present(0, 0));
	m_CurrBackBufferIndex = (m_CurrBackBufferIndex + 1) % s_SwapChainBufferCount;

	// Advance the fence value to mark commands up to this fence point.
	m_CurrFrameResource->m_FenceValue = ++m_CurrentFence;

	// Add an instruction to the command queue to set a new fence point. 
	// Because we are on the GPU timeline, the new fence point won't be 
	// set until the GPU finishes processing all the commands prior to this Signal().
	m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence);
}

Unique<DX12PipelineState> DX12RHI::CreatePSO(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc, const V_Array<D3D12_INPUT_ELEMENT_DESC>* layout, Shader* vsShader, Shader* psShader)
{
	/*if (layout)
	{
		desc.InputLayout = { layout->data(), (UINT)layout->size() };
	}

	if (vsShader)
	{
		auto dx12VsShader = dynamic_cast<DX12Shader*>(vsShader);
		desc.VS = { GetVSShaderBufferPointer(dx12VsShader), GetVSShaderBufferSize(dx12VsShader) };
	}

	if (psShader)
	{
		auto dx12PsShader = dynamic_cast<DX12Shader*>(psShader);
		desc.PS = { GetPSShaderBufferPointer(dx12PsShader), GetPSShaderBufferSize(dx12PsShader) };
	}*/

	return MakeUnique<DX12PipelineState>(m_Device.Get(), desc);
}

Unique<DX12VertexBuffer> DX12RHI::CreateVertexBuffer(void* vertices, uint32_t byteSize, uint32_t elementSize, const V_Array<D3D12_INPUT_ELEMENT_DESC>* layout)
{
	return MakeUnique<DX12VertexBuffer>(m_Device.Get(), m_CommandList.Get(), vertices, byteSize, elementSize, layout);
}

Unique<DX12VertexBuffer> DX12RHI::CreateUploadVertexBuffer(void* vertices, uint32_t byteSize, uint32_t elementSize, const V_Array<D3D12_INPUT_ELEMENT_DESC>* layout)
{
	return MakeUnique<DX12VertexBuffer>(m_Device.Get(), vertices, byteSize, elementSize, layout);
}

Unique<DX12IndexBuffer> DX12RHI::CreateIndexBuffer(void* indices, uint32_t count, ShaderDataType indiceDataType)
{
	return MakeUnique<DX12IndexBuffer>(m_Device.Get(), m_CommandList.Get(), indices, count, indiceDataType);
}

Unique<DX12IndexBuffer> DX12RHI::CreateUploadIndexBuffer(void* indices, uint32 count, ShaderDataType indiceDataType)
{
	return MakeUnique<DX12IndexBuffer>(m_Device.Get(), indices, count, indiceDataType);
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

	m_CurrBackBufferIndex = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < s_SwapChainBufferCount; i++)
	{
		ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_SwapChainBuffer[i])));
		m_Device->CreateRenderTargetView(m_SwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, DescriptorUtils::GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));//m_RtvDescriptorSize);
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

	m_ScreenViewport.TopLeftX = 0;
	m_ScreenViewport.TopLeftY = 0;
	m_ScreenViewport.Width = static_cast<float>(param.WindowWidth);
	m_ScreenViewport.Height = static_cast<float>(param.WindowHeight);
	m_ScreenViewport.MinDepth = 0.0f;
	m_ScreenViewport.MaxDepth = 1.0f;

	m_ScissorRect = { 0, 0, param.WindowWidth, param.WindowHeight };

	m_PrespectiveCamera.SetLens(0.25f * PI, (float)param.WindowWidth / param.WindowHeight, 1.0f, 1000.0f);
	if (m_Ssao != nullptr)
	{
		m_Ssao->OnResize(param.WindowWidth, param.WindowHeight);

		m_Ssao->RebuildDescriptors(m_DepthStencilBuffer.Get());
	}
}

void DX12RHI::InitializeBase(const RHIInitializeParam& param)
{
#if defined(DEBUG) || defined(_DEBUG)
	// Enable the D3D12 debug layer.
	{
		ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_Factory)));

	EnumAdaptersAndCreateDevice();

	CreateFence();

	CheckQualityLevel();

	for (int i = 0; i < _countof(m_DynamicDescriptorHeaps); i++)
	{
		m_DynamicDescriptorHeaps[i] = MakeUnique<DX12DynamicDescriptorHeap>();
	}

	CreateCommandQueue();

	CreateSwapChain();

	CreateRtvAndDsvHeaps();

	Resize(param);
}

//void DX12RHI::LoadSkinnedModel()
//{
//	std::vector<M3DLoader::DX12SkinnedVertex> vertices;
//	std::vector<std::uint16_t> indices;
//
//	M3DLoader m3dLoader;
//	m3dLoader.LoadM3d(DEFAULT_MODEL_PATH_3, vertices, indices,
//		mSkinnedSubsets, mSkinnedMats, mSkinnedInfo);
//
//	mSkinnedModelInst = std::make_unique<SkinnedModelInstance>();
//	mSkinnedModelInst->SkinnedInfo = &mSkinnedInfo;
//	mSkinnedModelInst->FinalTransforms.resize(mSkinnedInfo.BoneCount());
//	mSkinnedModelInst->ClipName = "Take1";
//	mSkinnedModelInst->TimePos = 0.0f;
//
//	const UINT vbByteSize = (UINT)vertices.size() * sizeof(DX12SkinnedVertex);
//	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);
//
//	auto geo = std::make_unique<MeshGeometry>();
//	geo->m_Name = "soldier";
//	geo->CopyCPUData(vertices, indices);
//	geo->SetVertexBufferAndIndexBuffer(CreateVertexBuffer((float*)vertices.data(), vbByteSize, (UINT)sizeof(DX12SkinnedVertex), &m_InputLayers["skinnedDefault"]),
//		CreateIndexBuffer(indices.data(), (UINT)indices.size(), ShaderDataType::Short));
//
//	for (UINT i = 0; i < (UINT)mSkinnedSubsets.size(); ++i)
//	{
//		SubmeshGeometry submesh;
//		std::string name = "sm_" + std::to_string(i);
//
//		submesh.IndexCount = (UINT)mSkinnedSubsets[i].FaceCount * 3;
//		submesh.StartIndexLocation = mSkinnedSubsets[i].FaceStart * 3;
//		submesh.BaseVertexLocation = 0;
//
//		geo->DrawArgs[name] = submesh;
//	}
//
//	m_Geometries[geo->m_Name] = std::move(geo);
//}

void DX12RHI::InitBaseGeometry()
{
	GeometryGenerator::CreateBoxStaticMesh(1.0f, 1.0f, 1.0f, 3);
	GeometryGenerator::CreateSphereStaticMesh(0.5f, 20, 20);
}

void DX12RHI::InitBaseTexture()
{
	m_Fonts["default"] = MakeUnique<FontAtlas>(DEFAULT_FONT_PATH_1, 16.0f);
	Texture::CreateTexture("font", m_Fonts["default"]->GetTextureWidth(), m_Fonts["default"]->GetTextureHeight(), m_Fonts["default"]->GetTextureDataRGBA32());

	//CreateTexture("font", m_Fonts["default"]->GetTextureWidth(), m_Fonts["default"]->GetTextureHeight(), m_Fonts["default"]->GetTextureDataRGBA32());

	Texture::CreateTexture("bricksDiffuseMap", DEFAULT_TEXTURE_PATH_13);
	Texture::CreateTexture("bricksNormalMap", DEFAULT_TEXTURE_PATH_14);
	Texture::CreateTexture("tileDiffuseMap", DEFAULT_TEXTURE_PATH_3);
	Texture::CreateTexture("tileNormalMap", DEFAULT_TEXTURE_PATH_15);
	/*CreateTexture("defaultDiffuseMap", DEFAULT_TEXTURE_PATH_10);
	CreateTexture("defaultNormalMap", DEFAULT_TEXTURE_PATH_16);*/
	Texture::CreateTexture("skyCubeMap", DEFAULT_TEXTURE_PATH_18);

	/*for (UINT i = 0; i < mSkinnedMats.size(); ++i)
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
	}*/
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

	//m_Shaders["standardVS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_DEFAULT, { nullptr, nullptr }, { "VS", "PS"}, "vs_5_1");
	//m_Shaders["skinnedVS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_1, skinnedDefines, "VS", "vs_5_1");
	//m_Shaders["opaquePS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_DEFAULT, nullptr, "PS", "ps_5_1");

	//m_Shaders["rawVS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_DEFAULT_RAW, nullptr, "VS", "vs_5_1");
	//m_Shaders["rawPS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_DEFAULT_RAW, nullptr, "PS", "ps_5_1");

	/*m_Shaders["shadowVS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_3, nullptr, "VS", "vs_5_1");
	m_Shaders["skinnedShadowVS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_3, skinnedDefines, "VS", "vs_5_1");
	m_Shaders["shadowOpaquePS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_3, nullptr, "PS", "ps_5_1");
	m_Shaders["shadowAlphaTestedPS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_3, alphaTestDefines, "PS", "ps_5_1");

	m_Shaders["debugVS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_4, nullptr, "VS", "vs_5_1"); 
	m_Shaders["debugPS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_4, nullptr, "PS", "ps_5_1"); 

	m_Shaders["drawNormalsVS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_6, nullptr, "VS", "vs_5_1"); 
	m_Shaders["skinnedDrawNormalsVS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_6, skinnedDefines, "VS", "vs_5_1"); 
	m_Shaders["drawNormalsPS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_6, nullptr, "PS", "ps_5_1"); 

	m_Shaders["ssaoVS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_5, nullptr, "VS", "vs_5_1"); 
	m_Shaders["ssaoPS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_5, nullptr, "PS", "ps_5_1"); 

	m_Shaders["ssaoBlurVS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_7, nullptr, "VS", "vs_5_1"); 
	m_Shaders["ssaoBlurPS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_7, nullptr, "PS", "ps_5_1"); 

	m_Shaders["skyVS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_2, nullptr, "VS", "vs_5_1"); 
	m_Shaders["skyPS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_2, nullptr, "PS", "ps_5_1"); */

	//m_Shaders["uiVS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_UI, nullptr, "VS", "vs_5_1");
	//m_Shaders["uiPS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_UI, nullptr, "PS", "ps_5_1");

	m_InputLayers["default"] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	m_InputLayers["defaultRaw"] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	m_InputLayers["skinnedDefault"] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BONEINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 56, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	m_InputLayers["ui"] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,   0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	Shader::CreateShader("opaque", DEFAULT_SHADER_PATH_DEFAULT);
	Shader::CreateShader("ui", DEFAULT_SHADER_PATH_UI);
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

		auto staticSamplers = GetStaticSamplers();

		// A root signature is an array of root parameters.
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(6, slotRootParameter,
			(UINT)staticSamplers.size(), staticSamplers.data(),
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		m_RootSignatures["default"] = MakeUnique<DX12RootSignature>(m_Device.Get(), rootSigDesc);
	}

	//{
	//	CD3DX12_DESCRIPTOR_RANGE texTable0;
	//	texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0);

	//	CD3DX12_DESCRIPTOR_RANGE texTable1;
	//	texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0);

	//	// Root parameter can be a table, root descriptor or root constants.
	//	CD3DX12_ROOT_PARAMETER slotRootParameter[4];

	//	// Perfomance TIP: Order from most frequent to least frequent.
	//	slotRootParameter[0].InitAsConstantBufferView(0);
	//	slotRootParameter[1].InitAsConstants(1, 1);
	//	slotRootParameter[2].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);
	//	slotRootParameter[3].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);

	//	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
	//		0, // shaderRegister
	//		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
	//		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
	//		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
	//		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	//	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
	//		1, // shaderRegister
	//		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
	//		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
	//		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
	//		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	//	const CD3DX12_STATIC_SAMPLER_DESC depthMapSam(
	//		2, // shaderRegister
	//		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
	//		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
	//		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
	//		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
	//		0.0f,
	//		0,
	//		D3D12_COMPARISON_FUNC_LESS_EQUAL,
	//		D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE);

	//	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
	//		3, // shaderRegister
	//		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
	//		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
	//		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
	//		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	//	std::array<CD3DX12_STATIC_SAMPLER_DESC, 4> staticSamplers =
	//	{
	//		pointClamp, linearClamp, depthMapSam, linearWrap
	//	};

	//	// A root signature is an array of root parameters.
	//	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter,
	//		(UINT)staticSamplers.size(), staticSamplers.data(),
	//		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	//	m_RootSignatures["ssao"] = MakeUnique<DX12RootSignature>(m_Device.Get(), rootSigDesc);
	//}

	{
		CD3DX12_DESCRIPTOR_RANGE table;
		table.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, 0);

		CD3DX12_ROOT_PARAMETER slotRootParameter[2];
		slotRootParameter[0].InitAsConstants(16, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
		slotRootParameter[1].InitAsDescriptorTable(1, &table, D3D12_SHADER_VISIBILITY_PIXEL);

		CD3DX12_STATIC_SAMPLER_DESC staticSampler = {};
		staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		staticSampler.MipLODBias = 0.f;
		staticSampler.MaxAnisotropy = 0;
		staticSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		staticSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		staticSampler.MinLOD = 0.f;
		staticSampler.MaxLOD = 0.f;
		staticSampler.ShaderRegister = 0;
		staticSampler.RegisterSpace = 0;
		staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		CD3DX12_ROOT_SIGNATURE_DESC desc(_countof(slotRootParameter),slotRootParameter,1, &staticSampler,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

		m_RootSignatures["ui"] = MakeUnique<DX12RootSignature>(m_Device.Get(), desc);
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
	opaquePsoDesc.RTVFormats[0] = m_BackBufferFormat;
	opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = m_DepthStencilFormat;
	m_PSOs["opaque"] = CreatePSO(opaquePsoDesc, &m_InputLayers["default"], m_Shaders["standardVS"].get(), m_Shaders["opaquePS"].get());

	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueRawPsoDesc = opaquePsoDesc;
	m_PSOs["opaqueRaw"] = CreatePSO(opaqueRawPsoDesc, &m_InputLayers["defaultRaw"], m_Shaders["rawVS"].get(), m_Shaders["rawPS"].get());

	//D3D12_GRAPHICS_PIPELINE_STATE_DESC skinnedOpaquePsoDesc = opaquePsoDesc;
	//m_PSOs["skinnedOpaque"] = CreatePSO(skinnedOpaquePsoDesc, &m_InputLayers["skinnedDefault"], m_Shaders["skinnedVS"].get(), m_Shaders["opaquePS"].get());

	//D3D12_GRAPHICS_PIPELINE_STATE_DESC smapPsoDesc = opaquePsoDesc;
	//smapPsoDesc.RasterizerState.DepthBias = 100000;
	//smapPsoDesc.RasterizerState.DepthBiasClamp = 0.0f;
	//smapPsoDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;
	//smapPsoDesc.pRootSignature = m_RootSignatures["default"]->GetRootSignature();

	//// Shadow map pass does not have a render target.
	//smapPsoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
	//smapPsoDesc.NumRenderTargets = 0;
	//m_PSOs["shadow_opaque"] = CreatePSO(smapPsoDesc, nullptr, m_Shaders["shadowVS"].get(), m_Shaders["shadowOpaquePS"].get());

	//D3D12_GRAPHICS_PIPELINE_STATE_DESC skinnedSmapPsoDesc = smapPsoDesc;
	//m_PSOs["skinnedShadow_opaque"] = CreatePSO(skinnedSmapPsoDesc, &m_InputLayers["skinnedDefault"], m_Shaders["skinnedShadowVS"].get(), m_Shaders["shadowOpaquePS"].get());

	//D3D12_GRAPHICS_PIPELINE_STATE_DESC debugPsoDesc = opaquePsoDesc;
	//debugPsoDesc.pRootSignature = m_RootSignatures["default"]->GetRootSignature();
	//
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
	//	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
	//		debugPsoDesc.BlendState.RenderTarget[i] = transparencyBlendDesc;
	//}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC uiPsoDesc = opaquePsoDesc;
	m_PSOs["ui"] = CreatePSO(uiPsoDesc, &m_InputLayers["ui"], m_Shaders["uiVS"].get(), m_Shaders["uiPS"].get());

	//D3D12_GRAPHICS_PIPELINE_STATE_DESC drawNormalsPsoDesc = opaquePsoDesc;
	//drawNormalsPsoDesc.RTVFormats[0] = Ssao::NormalMapFormat;
	//drawNormalsPsoDesc.SampleDesc.Count = 1;
	//drawNormalsPsoDesc.SampleDesc.Quality = 0;
	//drawNormalsPsoDesc.DSVFormat = m_DepthStencilFormat;
	//m_PSOs["drawNormals"] = CreatePSO(drawNormalsPsoDesc, nullptr, m_Shaders["drawNormalsVS"].get(), m_Shaders["drawNormalsPS"].get());

	//D3D12_GRAPHICS_PIPELINE_STATE_DESC skinnedDrawNormalsPsoDesc = drawNormalsPsoDesc;
	//m_PSOs["skinnedDrawNormals"] = CreatePSO(skinnedDrawNormalsPsoDesc, &m_InputLayers["skinnedDefault"], m_Shaders["skinnedDrawNormalsVS"].get(), m_Shaders["drawNormalsPS"].get());

	//D3D12_GRAPHICS_PIPELINE_STATE_DESC ssaoPsoDesc = opaquePsoDesc;
	//ssaoPsoDesc.InputLayout = { nullptr, 0 };
	//ssaoPsoDesc.pRootSignature = m_RootSignatures["ssao"]->GetRootSignature();

	//// SSAO effect does not need the depth buffer.
	//ssaoPsoDesc.DepthStencilState.DepthEnable = false;
	//ssaoPsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	//ssaoPsoDesc.RTVFormats[0] = Ssao::AmbientMapFormat;
	//ssaoPsoDesc.SampleDesc.Count = 1;
	//ssaoPsoDesc.SampleDesc.Quality = 0;
	//ssaoPsoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
	//m_PSOs["ssao"] = CreatePSO(ssaoPsoDesc, nullptr, m_Shaders["ssaoVS"].get(), m_Shaders["ssaoPS"].get());

	//D3D12_GRAPHICS_PIPELINE_STATE_DESC ssaoBlurPsoDesc = ssaoPsoDesc;
	//m_PSOs["ssaoBlur"] = CreatePSO(ssaoBlurPsoDesc, nullptr, m_Shaders["ssaoBlurVS"].get(), m_Shaders["ssaoBlurPS"].get());

	D3D12_GRAPHICS_PIPELINE_STATE_DESC skyPsoDesc = opaquePsoDesc;

	// The camera is inside the sky sphere, so just turn off culling.
	skyPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	// Make sure the depth function is LESS_EQUAL and not just LESS.  
	// Otherwise, the normalized depth values at z = 1 (NDC) will 
	// fail the depth test if the depth buffer was cleared to 1.
	skyPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	skyPsoDesc.pRootSignature = m_RootSignatures["default"]->GetRootSignature();
	m_PSOs["sky"] = CreatePSO(skyPsoDesc, nullptr, m_Shaders["skyVS"].get(), m_Shaders["skyPS"].get());

	/*D3D12_GRAPHICS_PIPELINE_STATE_DESC uiPsoDesc;
	ZeroMemory(&uiPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	uiPsoDesc.pRootSignature = m_RootSignatures["ui"]->GetRootSignature();
	uiPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	uiPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	uiPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	uiPsoDesc.SampleMask = UINT_MAX;
	uiPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	uiPsoDesc.NumRenderTargets = 1;
	uiPsoDesc.RTVFormats[0] = mBackBufferFormat;
	uiPsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1; 
	uiPsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	uiPsoDesc.DSVFormat = mDepthStencilFormat;

	m_PSOs["ui"] = CreatePSO(uiPsoDesc, &m_InputLayers["ui"], mShaders["uiVS"].get(), mShaders["uiPS"].get());*/
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
	sd.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	sd.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaState - 1) : 0;
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
	for (auto ri : m_RitemLayer[(int)RenderLayer::Opaque])
	{
		auto geo = ri->Geo;

		// Skip invisible render-items.
		/*if (ri->Visible == false)
			continue;*/

		DirectX::XMMATRIX W = XMLoadFloat4x4((const XMFLOAT4X4*)(&ri->World));
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

void DX12RHI::WaitFence()
{
	if (m_CurrFrameResource->m_FenceValue != 0 && m_Fence->GetCompletedValue() < m_CurrFrameResource->m_FenceValue)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

		ThrowIfFailed(m_Fence->SetEventOnCompletion(m_CurrFrameResource->m_FenceValue, eventHandle));

		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void DX12RHI::ResetCmdListAlloc()
{
	auto cmdListAlloc = m_CurrFrameResource->m_CommandAlloc;
	ThrowIfFailed(cmdListAlloc->Reset());
	ThrowIfFailed(m_CommandList->Reset(cmdListAlloc.Get(), nullptr));
}

void DX12RHI::ExecuteCmdList(bool isSwapChain)
{

	/*ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	m_CurrFrameResource->m_FenceValue = ++m_CurrentFence;
	m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence);*/



	// Done recording commands.
	ThrowIfFailed(m_CommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	if (isSwapChain)
	{
		// Swap the back and front buffers
		ThrowIfFailed(m_SwapChain->Present(0, 0));
		m_CurrBackBufferIndex = (m_CurrBackBufferIndex + 1) % s_SwapChainBufferCount;
	}

	
	// Advance the fence value to mark commands up to this fence point.
	m_CurrFrameResource->m_FenceValue = ++m_CurrentFence;

	// Add an instruction to the command queue to set a new fence point. 
	// Because we are on the GPU timeline, the new fence point won't be 
	// set until the GPU finishes processing all the commands prior to this Signal().
	m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence);
}

void DX12RHI::CreateRtvAndDsvHeaps()
{
	m_RtvDescriptorHeap = DX12DescriptorAllocator::GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, s_SwapChainBufferCount + 3);
	m_DsvHeap = DX12DescriptorAllocator::GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 2);
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

	m4xMsaaQuality = msQualityLevels.NumQualityLevels;
	assert(m4xMsaaQuality > 0 && "Unexpected MSAA quality level.");
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
	auto& currObjectCB = m_CurrFrameResource->m_ObjectUniform;

	//for (auto& e : m_RenderItems)
	//{
	//	if (e.NumFramesDirty > 0)
	//	{
	//		DirectX::XMMATRIX world = XMLoadFloat4x4((const XMFLOAT4X4*)(&e.World));
	//		DirectX::XMMATRIX texTransform = XMLoadFloat4x4((const XMFLOAT4X4*)(&e.TexTransform));

	//		ObjectUniform objConstants;
	//		XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(world));
	//		XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
	//		/*objConstants.MaterialIndex = e.Mat->MatCBIndex;

	//		currObjectCB->CopyData(e->ObjCBIndex, objConstants);*/

	//		// Next FrameResource need to be updated too.
	//		e.NumFramesDirty--;
	//	}
	//}
}

void DX12RHI::UpdateSkinnedCBs(const RiverTime& time)
{
	auto currSkinnedCB = m_CurrFrameResource->m_SkinnedUniform.get();

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
	auto currMaterialCB = m_CurrFrameResource->m_MaterialUniform.get();
	for (auto& e : m_Materials)
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
			matData.DiffuseMapIndex = mat->m_DiffuseTexture->GetTextureId(); //mat->DiffuseSrvHeapIndex;
			matData.NormalMapIndex = mat->m_NormalTexture->GetTextureId(); //mat->NormalSrvHeapIndex;

			currMaterialCB->CopyData(mat->MatCBIndex, matData);

			// Next FrameResource need to be updated too.
			mat->NumFramesDirty--;
		}
	}
}

void DX12RHI::UpdateMainPass(/*const RiverTime& time*/)
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

	XMStoreFloat4x4(&m_MainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&m_MainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&m_MainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&m_MainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&m_MainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&m_MainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	XMStoreFloat4x4(&m_MainPassCB.ViewProjTex, XMMatrixTranspose(viewProjTex));
	XMStoreFloat4x4(&m_MainPassCB.ShadowTransform, XMMatrixTranspose(shadowTransform));
	m_MainPassCB.EyePosW = m_PrespectiveCamera.GetPosition();
	m_MainPassCB.RenderTargetSize = XMFLOAT2((float)m_InitParam.WindowWidth, (float)m_InitParam.WindowHeight);
	m_MainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / m_InitParam.WindowWidth, 1.0f / m_InitParam.WindowHeight);
	m_MainPassCB.NearZ = 1.0f;
	m_MainPassCB.FarZ = 1000.0f;
	m_MainPassCB.TotalTime = 0;//time.TotalTime();
	m_MainPassCB.DeltaTime = 0;//time.DeltaTime();
	m_MainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	m_MainPassCB.Lights[0].Direction = mRotatedLightDirections[0];
	m_MainPassCB.Lights[0].Strength = { 0.9f, 0.9f, 0.7f };
	m_MainPassCB.Lights[1].Direction = mRotatedLightDirections[1];
	m_MainPassCB.Lights[1].Strength = { 0.4f, 0.4f, 0.4f };
	m_MainPassCB.Lights[2].Direction = mRotatedLightDirections[2];
	m_MainPassCB.Lights[2].Strength = { 0.2f, 0.2f, 0.2f };

	auto currPassCB = m_CurrFrameResource->m_PassUniform.get();
	currPassCB->CopyData(0, m_MainPassCB);
}

void DX12RHI::UpdateShadowPass(const RiverTime& time)
{
	XMMATRIX view = XMLoadFloat4x4(&mLightView);
	XMMATRIX proj = XMLoadFloat4x4(&mLightProj);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	UINT w = m_ShadowMap->Width();
	UINT h = m_ShadowMap->Height();

	XMStoreFloat4x4(&m_ShadowPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&m_ShadowPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&m_ShadowPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&m_ShadowPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&m_ShadowPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&m_ShadowPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	m_ShadowPassCB.EyePosW = mLightPosW;
	m_ShadowPassCB.RenderTargetSize = XMFLOAT2((float)w, (float)h);
	m_ShadowPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / w, 1.0f / h);
	m_ShadowPassCB.NearZ = mLightNearZ;
	m_ShadowPassCB.FarZ = mLightFarZ;

	auto currPassCB = m_CurrFrameResource->m_PassUniform.get();
	currPassCB->CopyData(1, m_ShadowPassCB);
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

	ssaoCB.Proj = m_MainPassCB.Proj;
	ssaoCB.InvProj = m_MainPassCB.InvProj;
	XMStoreFloat4x4(&ssaoCB.ProjTex, XMMatrixTranspose(P * T));

	m_Ssao->GetOffsetVectors(ssaoCB.OffsetVectors);

	auto blurWeights = m_Ssao->CalcGaussWeights(2.5f);
	ssaoCB.BlurWeights[0] = XMFLOAT4(&blurWeights[0]);
	ssaoCB.BlurWeights[1] = XMFLOAT4(&blurWeights[4]);
	ssaoCB.BlurWeights[2] = XMFLOAT4(&blurWeights[8]);

	ssaoCB.InvRenderTargetSize = XMFLOAT2(1.0f / m_Ssao->SsaoMapWidth(), 1.0f / m_Ssao->SsaoMapHeight());

	// Coordinates given in view space.
	ssaoCB.OcclusionRadius = 0.5f;
	ssaoCB.OcclusionFadeStart = 0.2f;
	ssaoCB.OcclusionFadeEnd = 2.0f;
	ssaoCB.SurfaceEpsilon = 0.05f;

	auto currSsaoCB = m_CurrFrameResource->m_SsaoUniform.get();
	currSsaoCB->CopyData(0, ssaoCB);
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
		m_RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		m_CurrBackBufferIndex,
		DescriptorUtils::GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));//m_RtvDescriptorSize);
}

void DX12RHI::InitBaseMaterials()
{
	auto mat = Material::CreateMaterial("uiDefault");
	auto shader = AssetManager::Get()->GetShader("ui");
	mat->InitBaseParam(MaterialBlendMode::Opaque, shader, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.1f, 0.1f, 0.1f }, 1.0f, 3, nullptr, nullptr);
}

void DX12RHI::InitFrameBuffer()
{
	for (int i = 0; i < RHI::GetFrameCount(); ++i)
	{
		m_FrameBuffer.push_back(MakeUnique<DX12FrameBuffer>(m_Device.Get(), 2, GetRenderItemMaxCount()/*(UINT)m_AllRitems.size()*/,
			1, GetMaterialMaxCount()));
	}

	m_CurrFrameResourceIndex = (m_CurrFrameResourceIndex + 1) % RHI::GetFrameCount();
	m_CurrFrameResource = m_FrameBuffer[m_CurrFrameResourceIndex].get();
}

void DX12RHI::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const V_Array<DX12RenderItem*>& ritems)
{
	UINT objCBByteSize = RendererUtil::CalcMinimumGPUAllocSize(sizeof(ObjectUniform));
	UINT skinnedCBByteSize = RendererUtil::CalcMinimumGPUAllocSize(sizeof(SkinnedUniform));

	auto objectCB = m_CurrFrameResource->m_ObjectUniform->Resource();
	auto skinnedCB = m_CurrFrameResource->m_SkinnedUniform->Resource();

	// For each render item...
	for (size_t i = 0; i < ritems.size(); ++i)
	{
		auto ri = ritems[i];

		cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
		cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
		cmdList->IASetPrimitiveTopology(GetRenderItemPrimtiveType(ri->PriType));

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
	m_CommandList->RSSetViewports(1, &m_ShadowMap->Viewport());
	m_CommandList->RSSetScissorRects(1, &m_ShadowMap->ScissorRect());

	// Change to DEPTH_WRITE.
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_ShadowMap->Resource(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	// Clear the back buffer and depth buffer.
	m_CommandList->ClearDepthStencilView(m_ShadowMap->Dsv(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	m_CommandList->OMSetRenderTargets(0, nullptr, false, &m_ShadowMap->Dsv());

	// Bind the pass constant buffer for the shadow map pass.
	UINT passCBByteSize = RendererUtil::CalcMinimumGPUAllocSize(sizeof(PassUniform));
	auto passCB = m_CurrFrameResource->m_PassUniform->Resource();
	D3D12_GPU_VIRTUAL_ADDRESS passCBAddress = passCB->GetGPUVirtualAddress() + 1 * passCBByteSize;
	m_CommandList->SetGraphicsRootConstantBufferView(2, passCBAddress);

	//mCommandList->SetPipelineState(mPSOs["shadow_opaque"].Get());
	m_CommandList->SetPipelineState(m_PSOs["shadow_opaque"]->GetPSO());
	DrawRenderItems(m_CommandList.Get(), m_RitemLayer[(int)RenderLayer::Opaque]);

	//mCommandList->SetPipelineState(mPSOs["skinnedShadow_opaque"].Get());
	m_CommandList->SetPipelineState(m_PSOs["skinnedShadow_opaque"]->GetPSO());
	DrawRenderItems(m_CommandList.Get(), m_RitemLayer[(int)RenderLayer::SkinnedOpaque]);

	// Change back to GENERIC_READ so we can read the texture in a shader.
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_ShadowMap->Resource(),
		D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));
}

void DX12RHI::DrawNormalsAndDepth()
{
	m_CommandList->RSSetViewports(1, &m_ScreenViewport);
	m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

	auto normalMap = m_Ssao->NormalMap();
	auto normalMapRtv = m_Ssao->NormalMapRtv();

	// Change to RENDER_TARGET.
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(normalMap,
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Clear the screen normal map and depth buffer.
	float clearValue[] = { 0.0f, 0.0f, 1.0f, 0.0f };
	m_CommandList->ClearRenderTargetView(normalMapRtv, clearValue, 0, nullptr);
	m_CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	m_CommandList->OMSetRenderTargets(1, &normalMapRtv, true, &DepthStencilView());

	// Bind the constant buffer for this pass.
	auto passCB = m_CurrFrameResource->m_PassUniform->Resource();
	m_CommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	//mCommandList->SetPipelineState(mPSOs["drawNormals"].Get());
	m_CommandList->SetPipelineState(m_PSOs["drawNormals"]->GetPSO());
	DrawRenderItems(m_CommandList.Get(), m_RitemLayer[(int)RenderLayer::Opaque]);

	//mCommandList->SetPipelineState(mPSOs["skinnedDrawNormals"].Get());
	m_CommandList->SetPipelineState(m_PSOs["skinnedDrawNormals"]->GetPSO());
	DrawRenderItems(m_CommandList.Get(), m_RitemLayer[(int)RenderLayer::SkinnedOpaque]);

	// Change back to GENERIC_READ so we can read the texture in a shader.
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(normalMap,
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
}

void DX12RHI::DrawUI()
{

}

void DX12RHI::CreateSRV(CD3DX12_CPU_DESCRIPTOR_HANDLE& handle, ID3D12Resource* textureRes, D3D12_SHADER_RESOURCE_VIEW_DESC& desc, uint32_t handleOffset)
{
	if (handleOffset > 0)
	{
		handle.Offset(handleOffset, DescriptorUtils::GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	}

	return m_Device->CreateShaderResourceView(textureRes, &desc, handle);
}
