#include "RiverPch.h"
#include "RendererUtil.h"

#include "Renderer/Font/Header/FontAtlas.h"
#include "Renderer/Font/Header/Font.h"
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
#include "Renderer/DX12Renderer/Header/DX12DescriptorAllocator.h"

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

		m_Ssao->SetPSOs(m_PSOs["ssao"]->GetPSO(), m_PSOs["ssaoBlur"]->GetPSO());
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

	m_CurrFrameResourceIndex = (m_CurrFrameResourceIndex + 1) % s_FrameBufferCount;
	m_CurrFrameResource = m_FrameBuffer[m_CurrFrameResourceIndex].get();
	WaitFence();

	AnimationMaterials(time);
	UpdateObjectCBs();
	UpdateSkinnedCBs(time);
	UpdateMaterialCBs();
	//UpdateShadowTransform(time);
	UpdateMainPass(time);
	UpdateShadowPass(time);
	UpdateSsaoCBs(time);
}

void DX12RHI::UpdateSceneData(const V_Array<RawVertex>& vertices, const V_Array<uint16_t> indices)
{
	{
		if (!m_CurrFrameResource)
		{
			return;
		}
		auto& currObjectCB = m_CurrFrameResource->m_ObjectUniform;
		int index = (int)m_AllRitems.size();
		for (size_t i = 0; i < m_RenderItems.size(); i++)
		{
			DirectX::XMMATRIX world = XMLoadFloat4x4((const XMFLOAT4X4*)(&m_RenderItems[i].World));
			DirectX::XMMATRIX texTransform = XMLoadFloat4x4((const XMFLOAT4X4*)(&m_RenderItems[i].TexTransform));

			ObjectUniform objConstants;
			XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(world));
			XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
			objConstants.MaterialIndex = 2;

			currObjectCB->CopyData(index++, objConstants);
		}
		
	}

	auto& geo = m_Geometries["scene"];
	UINT verticesMore = (UINT)vertices.size() + 1000;
	UINT indicesMore = (UINT)indices.size() + 3000;
	if (geo == nullptr)
	{
		const UINT vbByteSize = verticesMore * sizeof(RawVertex);
		const UINT ibByteSize = indicesMore * sizeof(std::uint16_t);

		auto geo = MakeUnique<MeshGeometry>();
		geo->m_Name = "shapeGeo";
		//geo->CopyCPUData(vertices, indices);
		geo->SetVertexBufferAndIndexBuffer(CreateUploadVertexBuffer((float*)vertices.data(), vbByteSize, (UINT)sizeof(RawVertex), &m_InputLayers["defaultRaw"]),
			CreateUploadIndexBuffer((void*)indices.data(), (UINT)indices.size(), ShaderDataType::Short));

		m_Geometries["scene"] = std::move(geo);
	}
	else
	{
		geo->m_VertexBuffer->UpdateData(m_Device.Get(), m_CommandList.Get(), (void*)vertices.data(), (uint32)vertices.size(), 1000);
		geo->m_IndexBuffer->UpdateData(m_Device.Get(), m_CommandList.Get(), (void*)indices.data(), (uint32)indices.size(), 3000);
	}
}

void DX12RHI::UpdateUIData(V_Array<UIVertex>& vertices, V_Array<uint16_t> indices)
{
	{
		if (!m_CurrFrameResource)
		{
			return;
		}
		auto& currObjectCB = m_CurrFrameResource->m_ObjectUniform;
		auto index = (int)m_AllRitems.size() + 2;
		for (size_t i = 0; i < m_UIRenderItems.size(); i++)
		{
			DirectX::XMMATRIX world = XMLoadFloat4x4((const XMFLOAT4X4*)(&m_UIRenderItems[i].World));

			ObjectUniform objConstants;
			XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(world));
			objConstants.MaterialIndex = 2;//m_Materials["bricks0"]->MatCBIndex;

			currObjectCB->CopyData(index++, objConstants);
		}

	}

	auto& geo = m_Geometries["ui"];
	UINT verticesMore = (UINT)vertices.size() + 1000;
	UINT indicesMore = (UINT)indices.size() + 3000;
	if (geo == nullptr)
	{
		const UINT vbByteSize = verticesMore * sizeof(UIVertex);
		const UINT ibByteSize = indicesMore * sizeof(std::uint16_t);

		auto geo = MakeUnique<MeshGeometry>();
		geo->m_Name = "shapeGeo";
		//geo->CopyCPUData(vertices, indices);
		geo->SetVertexBufferAndIndexBuffer(CreateUploadVertexBuffer((float*)vertices.data(), vbByteSize, (UINT)sizeof(UIVertex), &m_InputLayers["ui"]),
			CreateUploadIndexBuffer(indices.data(), (UINT)indices.size(), ShaderDataType::Short));

		m_Geometries["ui"] = std::move(geo);
	}
	else
	{
		geo->m_VertexBuffer->UpdateData(m_Device.Get(), m_CommandList.Get(), vertices.data(), (uint32)vertices.size(), 1000);
		geo->m_IndexBuffer->UpdateData(m_Device.Get(), m_CommandList.Get(), indices.data(), (uint32)indices.size(), 3000);
	}

	m_UIRenderItem->IndexCount = (int)indices.size();
}

DX12Texture* DX12RHI::CreateTexture(const char* name, const char* filePath)
{
	DX12Texture* ret = nullptr;
	if (name && filePath && m_Textures.find(name) == std::end(m_Textures))
	{
		auto texture = MakeUnique<DX12Texture>(m_Device.Get(), m_CommandList.Get(), name, filePath);
		ret = texture.get();
		m_Textures[name] = std::move(texture);
	}

	if (m_Textures.find(name) != std::end(m_Textures))
	{
		ret = m_Textures[name].get();
	}

	return ret;
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

DX12Texture* DX12RHI::CreateTexture(const char* name, int width, int height, const uint8* data)
{
	DX12Texture* ret = nullptr;
	if (name && data && m_Textures.find(name) == std::end(m_Textures))
	{
		auto texture = MakeUnique<DX12Texture>(m_Device.Get(), m_CommandList.Get(), name, data, width, height);
		ret = texture.get();
		m_Textures[name] = std::move(texture);
	}

	return ret;
}

void DX12RHI::RemoveTexture(const String& name)
{
	auto iter = m_Textures.find(name);
	if (iter != m_Textures.end())
	{
		m_Textures.erase(iter);
	}
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
	m_CommandList->SetGraphicsRootDescriptorTable(4, mNullSrv);

	//绑定纹理贴图，只需要提供起始位置，根签名能自动知道所需数据大小
	m_CommandList->SetGraphicsRootDescriptorTable(5, m_SrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	DrawSceneToShadowMap();

	DrawNormalsAndDepth();

	m_CommandList->SetGraphicsRootSignature(m_RootSignatures["ssao"]->GetRootSignature());
	m_Ssao->ComputeSsao(m_CommandList.Get(), m_CurrFrameResource, 2);

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

	CD3DX12_GPU_DESCRIPTOR_HANDLE skyTexDescriptor(m_SrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	skyTexDescriptor.Offset(m_Textures["skyCubeMap"]->GetTextureId(), DescriptorUtils::GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)); //m_CbvSrvUavDescriptorSize);
	m_CommandList->SetGraphicsRootDescriptorTable(4, skyTexDescriptor);

	m_CommandList->SetPipelineState(m_PSOs["opaque"]->GetPSO());
	DrawRenderItems(m_CommandList.Get(), m_RitemLayer[(int)RenderLayer::Opaque]);

	{
		m_CommandList->SetPipelineState(m_PSOs["opaqueRaw"]->GetPSO());
		auto& geo = m_Geometries["scene"];
		if (geo)
		{
			auto index = m_AllRitems.size();
			UINT objCBByteSize = RendererUtil::CalcMinimumGPUAllocSize(sizeof(ObjectUniform));
			auto objectCB = m_CurrFrameResource->m_ObjectUniform->Resource();

			for (size_t i = 0; i < m_RenderItems.size(); i++)
			{
				m_CommandList->IASetVertexBuffers(0, 1, &geo->VertexBufferView());
				m_CommandList->IASetIndexBuffer(&geo->IndexBufferView());
				m_CommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + index++ * objCBByteSize;

				m_CommandList->SetGraphicsRootConstantBufferView(0, objCBAddress);
				m_CommandList->SetGraphicsRootConstantBufferView(1, 0);
				m_CommandList->DrawIndexedInstanced(m_RenderItems[i].IndexCount, 	1, 
					m_RenderItems[i].StartIndexLocation, m_RenderItems[i].BaseVertexLocation, 0);
			}
		}
	}

	{
		m_CommandList->SetPipelineState(m_PSOs["debug"]->GetPSO());
		auto& geo = m_Geometries["ui"];
		if (geo)
		{
			auto index = 29;//m_AllRitems.size() + 2;
			UINT objCBByteSize = RendererUtil::CalcMinimumGPUAllocSize(sizeof(ObjectUniform));
			auto objectCB = m_CurrFrameResource->m_ObjectUniform->Resource();

			for (size_t i = 0; i < m_UIRenderItems.size(); i++)
			{
				m_CommandList->IASetVertexBuffers(0, 1, &geo->VertexBufferView());
				m_CommandList->IASetIndexBuffer(&geo->IndexBufferView());
				m_CommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + index * objCBByteSize;

				m_CommandList->SetGraphicsRootConstantBufferView(0, objCBAddress);
				m_CommandList->SetGraphicsRootConstantBufferView(1, 0);
				m_CommandList->DrawIndexedInstanced(m_UIRenderItems[i].IndexCount, 1,
					m_UIRenderItems[i].StartIndexLocation, m_UIRenderItems[i].BaseVertexLocation, 0);
			}
		}
	}

	//mCommandList->SetPipelineState(mPSOs["skinnedOpaque"].Get());
	m_CommandList->SetPipelineState(m_PSOs["skinnedOpaque"]->GetPSO());
	DrawRenderItems(m_CommandList.Get(), m_RitemLayer[(int)RenderLayer::SkinnedOpaque]);

	m_CommandList->SetPipelineState(m_PSOs["debug"]->GetPSO());
	DrawRenderItems(m_CommandList.Get(), m_RitemLayer[(int)RenderLayer::Debug]);

	m_CommandList->SetPipelineState(m_PSOs["sky"]->GetPSO());
	DrawRenderItems(m_CommandList.Get(), m_RitemLayer[(int)RenderLayer::Sky]);

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

Unique<DX12IndexBuffer> DX12RHI::CreateUploadIndexBuffer(void* indices, uint32_t count, ShaderDataType indiceDataType)
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
	geo->m_Name = "soldier";
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

	m_Geometries[geo->m_Name] = std::move(geo);
}

void DX12RHI::BuildShapeGeometry()
{
	DX12GeometryGenerator geoGen;
	DX12GeometryGenerator::MeshData box = geoGen.CreateBox1(1.0f, 1.0f, 1.0f, 3);
	DX12GeometryGenerator::MeshData grid = geoGen.CreateGrid1(20.0f, 30.0f, 60, 40);
	DX12GeometryGenerator::MeshData sphere = geoGen.CreateSphere1(0.5f, 20, 20);
	DX12GeometryGenerator::MeshData cylinder = geoGen.CreateCylinder1(0.5f, 0.3f, 3.0f, 20, 20);
	DX12GeometryGenerator::MeshData quad = geoGen.CreateQuad1(0.f, 0.f, 1.0f, 1.0f, 0.0f);
	DX12GeometryGenerator::MeshData uiQuad = geoGen.CreateBox1(5.0f, 5.0f, 1.0f, 3);

	UINT boxVertexOffset = 0;
	UINT gridVertexOffset = (UINT)box.Vertices.size();
	UINT sphereVertexOffset = gridVertexOffset + (UINT)grid.Vertices.size();
	UINT cylinderVertexOffset = sphereVertexOffset + (UINT)sphere.Vertices.size();
	UINT quadVertexOffset = cylinderVertexOffset + (UINT)cylinder.Vertices.size();
	UINT uiQuadVertexOffset = quadVertexOffset + (UINT)quad.Vertices.size();

	UINT boxIndexOffset = 0;
	UINT gridIndexOffset = (UINT)box.Indices32.size();
	UINT sphereIndexOffset = gridIndexOffset + (UINT)grid.Indices32.size();
	UINT cylinderIndexOffset = sphereIndexOffset + (UINT)sphere.Indices32.size();
	UINT quadIndexOffset = cylinderIndexOffset + (UINT)cylinder.Indices32.size();
	UINT uiQuadIndexOffset = quadIndexOffset + (UINT)quad.Indices32.size();

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

	SubmeshGeometry uiQuadSubmesh;
	uiQuadSubmesh.IndexCount = (UINT)uiQuad.Indices32.size();
	uiQuadSubmesh.StartIndexLocation = uiQuadVertexOffset;
	uiQuadSubmesh.BaseVertexLocation = uiQuadIndexOffset;

	auto totalVertexCount =
		box.Vertices.size() +
		grid.Vertices.size() +
		sphere.Vertices.size() +
		cylinder.Vertices.size() +
		quad.Vertices.size() + 
		uiQuad.Vertices.size();

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

	for (int i = 0; i < uiQuad.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = uiQuad.Vertices[i].Position;
		vertices[k].Normal = uiQuad.Vertices[i].Normal;
		vertices[k].TexC = uiQuad.Vertices[i].TexC;
		vertices[k].TangentU = uiQuad.Vertices[i].TangentU;
	}

	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
	indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));
	indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
	indices.insert(indices.end(), std::begin(cylinder.GetIndices16()), std::end(cylinder.GetIndices16()));
	indices.insert(indices.end(), std::begin(quad.GetIndices16()), std::end(quad.GetIndices16()));
	indices.insert(indices.end(), std::begin(uiQuad.GetIndices16()), std::end(uiQuad.GetIndices16()));

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(DX12Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->m_Name = "shapeGeo";
	geo->CopyCPUData(vertices, indices);
	geo->SetVertexBufferAndIndexBuffer(CreateVertexBuffer((float*)vertices.data(), vbByteSize, (UINT)sizeof(DX12Vertex), &m_InputLayers["skinnedDefault"]),
		CreateIndexBuffer(indices.data(), (UINT)indices.size(), ShaderDataType::Short));

	geo->DrawArgs["box"] = boxSubmesh;
	geo->DrawArgs["grid"] = gridSubmesh;
	geo->DrawArgs["sphere"] = sphereSubmesh;
	geo->DrawArgs["cylinder"] = cylinderSubmesh;
	geo->DrawArgs["quad"] = quadSubmesh;
	geo->DrawArgs["uiQuad"] = uiQuadSubmesh;

	m_Geometries[geo->m_Name] = std::move(geo);

	{
		auto geo = MakeUnique<MeshGeometry>();
		geo->m_Name = "ui";
		std::vector<UIVertex> vertices(50);
		std::vector<std::uint16_t> indices(100);
		//geo->CopyCPUData(vertices, indices);

		const UINT vbByteSize = (UINT)vertices.size() * sizeof(UIVertex);
		const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

		geo->SetVertexBufferAndIndexBuffer(CreateUploadVertexBuffer((float*)vertices.data(), vbByteSize, (UINT)sizeof(UIVertex), &m_InputLayers["ui"]),
			CreateUploadIndexBuffer(indices.data(), (UINT)indices.size(), ShaderDataType::Short));
		m_Geometries["ui"] = std::move(geo);
	}
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

	m_Fonts["default"] = MakeUnique<FontAtlas>(DEFAULT_FONT_PATH_1, 16.0f);
	CreateTexture("font", m_Fonts["default"]->GetTextureWidth(), m_Fonts["default"]->GetTextureHeight(), m_Fonts["default"]->GetTextureDataRGBA32());
}

void DX12RHI::InitBaseMaterials()
{
	auto bricks = MakeUnique<Material>("bricks0");
	bricks->InitBaseParam({ 1.0f, 1.0f, 1.0f, 1.0f }, { 0.1f, 0.1f, 0.1f }, 0.3f, 0, 
		m_Textures["bricksDiffuseMap"].get(), m_Textures["bricksNormalMap"].get());
	auto tile = MakeUnique<Material>("tile0");
	tile->InitBaseParam({ 0.9f, 0.9f, 0.9f, 1.0f }, { 0.2f, 0.2f, 0.2f }, 0.1f, 1, 
		m_Textures["tileDiffuseMap"].get(), m_Textures["tileNormalMap"].get());
	auto mirror = MakeUnique<Material>("mirror0");
	mirror->InitBaseParam({ 0.0f, 0.0f, 0.0f, 1.0f }, { 0.98f, 0.97f, 0.95f }, 0.1f, 2, 
		m_Textures["defaultDiffuseMap"].get(), m_Textures["defaultNormalMap"].get());
	auto sky = MakeUnique<Material>("sky");
	sky->InitBaseParam({ 1.0f, 1.0f, 1.0f, 1.0f }, { 0.1f, 0.1f, 0.1f }, 1.0f, 3, 
		m_Textures["skyCubeMap"].get(), m_Textures["skyCubeMap"].get());

	m_Materials["bricks0"] = std::move(bricks);
	m_Materials["tile0"] = std::move(tile);
	m_Materials["mirror0"] = std::move(mirror);
	m_Materials["sky"] = std::move(sky);

	UINT matCBIndex = 4;
	for (UINT i = 0; i < mSkinnedMats.size(); ++i)
	{
		auto mat = MakeUnique<Material>(mSkinnedMats[i].m_Name.c_str());
		mat->InitBaseParam(*(River::Float4*)(&mSkinnedMats[i].DiffuseAlbedo), *(River::Float3*)(&mSkinnedMats[i].FresnelR0), mSkinnedMats[i].Roughness,
			matCBIndex++, m_Textures[mSkinnedTextureNames[i * 2]].get(), m_Textures[mSkinnedTextureNames[i * 2 + 1]].get());//DiffuseSrvHeapIndex, NormalSrvHeapIndex);
		m_Materials[mSkinnedMats[i].m_Name] = std::move(mat);
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

	m_Shaders["standardVS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_1, nullptr, "VS", "vs_5_1");
	m_Shaders["skinnedVS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_1, skinnedDefines, "VS", "vs_5_1");
	m_Shaders["opaquePS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_1, nullptr, "PS", "ps_5_1");

	m_Shaders["rawVS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_DEFAULT_RAW, nullptr, "VS", "vs_5_1");
	m_Shaders["rawPS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_DEFAULT_RAW, nullptr, "PS", "ps_5_1");

	m_Shaders["shadowVS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_3, nullptr, "VS", "vs_5_1");
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
	m_Shaders["skyPS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_2, nullptr, "PS", "ps_5_1"); 

	m_Shaders["uiVS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_UI, nullptr, "VS", "vs_5_1");
	m_Shaders["uiPS"] = MakeUnique<DX12Shader>(DEFAULT_SHADER_PATH_UI, nullptr, "PS", "ps_5_1");

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

		m_RootSignatures["default"] = MakeUnique<DX12RootSignature>(m_Device.Get(), rootSigDesc);
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

		m_RootSignatures["ssao"] = MakeUnique<DX12RootSignature>(m_Device.Get(), rootSigDesc);
	}

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

	D3D12_GRAPHICS_PIPELINE_STATE_DESC skinnedOpaquePsoDesc = opaquePsoDesc;
	m_PSOs["skinnedOpaque"] = CreatePSO(skinnedOpaquePsoDesc, &m_InputLayers["skinnedDefault"], m_Shaders["skinnedVS"].get(), m_Shaders["opaquePS"].get());

	D3D12_GRAPHICS_PIPELINE_STATE_DESC smapPsoDesc = opaquePsoDesc;
	smapPsoDesc.RasterizerState.DepthBias = 100000;
	smapPsoDesc.RasterizerState.DepthBiasClamp = 0.0f;
	smapPsoDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;
	smapPsoDesc.pRootSignature = m_RootSignatures["default"]->GetRootSignature();

	// Shadow map pass does not have a render target.
	smapPsoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
	smapPsoDesc.NumRenderTargets = 0;
	m_PSOs["shadow_opaque"] = CreatePSO(smapPsoDesc, nullptr, m_Shaders["shadowVS"].get(), m_Shaders["shadowOpaquePS"].get());

	D3D12_GRAPHICS_PIPELINE_STATE_DESC skinnedSmapPsoDesc = smapPsoDesc;
	m_PSOs["skinnedShadow_opaque"] = CreatePSO(skinnedSmapPsoDesc, &m_InputLayers["skinnedDefault"], m_Shaders["skinnedShadowVS"].get(), m_Shaders["shadowOpaquePS"].get());

	D3D12_GRAPHICS_PIPELINE_STATE_DESC debugPsoDesc = opaquePsoDesc;
	debugPsoDesc.pRootSignature = m_RootSignatures["default"]->GetRootSignature();
	
	{
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
		for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
			debugPsoDesc.BlendState.RenderTarget[i] = transparencyBlendDesc;
	}

	m_PSOs["debug"] = CreatePSO(debugPsoDesc, &m_InputLayers["ui"], m_Shaders["uiVS"].get(), m_Shaders["uiPS"].get());

	D3D12_GRAPHICS_PIPELINE_STATE_DESC drawNormalsPsoDesc = opaquePsoDesc;
	drawNormalsPsoDesc.RTVFormats[0] = Ssao::NormalMapFormat;
	drawNormalsPsoDesc.SampleDesc.Count = 1;
	drawNormalsPsoDesc.SampleDesc.Quality = 0;
	drawNormalsPsoDesc.DSVFormat = m_DepthStencilFormat;
	m_PSOs["drawNormals"] = CreatePSO(drawNormalsPsoDesc, nullptr, m_Shaders["drawNormalsVS"].get(), m_Shaders["drawNormalsPS"].get());

	D3D12_GRAPHICS_PIPELINE_STATE_DESC skinnedDrawNormalsPsoDesc = drawNormalsPsoDesc;
	m_PSOs["skinnedDrawNormals"] = CreatePSO(skinnedDrawNormalsPsoDesc, &m_InputLayers["skinnedDefault"], m_Shaders["skinnedDrawNormalsVS"].get(), m_Shaders["drawNormalsPS"].get());

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
	m_PSOs["ssao"] = CreatePSO(ssaoPsoDesc, nullptr, m_Shaders["ssaoVS"].get(), m_Shaders["ssaoPS"].get());

	D3D12_GRAPHICS_PIPELINE_STATE_DESC ssaoBlurPsoDesc = ssaoPsoDesc;
	m_PSOs["ssaoBlur"] = CreatePSO(ssaoBlurPsoDesc, nullptr, m_Shaders["ssaoBlurVS"].get(), m_Shaders["ssaoBlurPS"].get());

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
	auto& currObjectCB = m_CurrFrameResource->m_ObjectUniform;

	for (auto& e : m_AllRitems)
	{
		if (e->NumFramesDirty > 0)
		{
			DirectX::XMMATRIX world = XMLoadFloat4x4((const XMFLOAT4X4*)(&e->World));
			DirectX::XMMATRIX texTransform = XMLoadFloat4x4((const XMFLOAT4X4*)(&e->TexTransform));

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
	m_MainPassCB.TotalTime = time.TotalTime();
	m_MainPassCB.DeltaTime = time.DeltaTime();
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

void DX12RHI::InitDescriptorHeaps()
{
	//
	// Create the SRV heap.
	//
	/*D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 64;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(m_Device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_SrvDescriptorHeap)));*/

	m_SrvDescriptorHeap = DX12DescriptorAllocator::GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//
	// Fill out the heap with actual descriptors.
	//
	

	std::vector<DX12Texture*> tex2DList =
	{
		m_Textures["bricksDiffuseMap"].get(),
		m_Textures["bricksNormalMap"].get(),
		m_Textures["tileDiffuseMap"].get(),
		m_Textures["tileNormalMap"].get(),
		m_Textures["defaultDiffuseMap"].get(),
		m_Textures["defaultNormalMap"].get(),
		m_Textures["font"].get(),
	};

	for (UINT i = 0; i < (UINT)mSkinnedTextureNames.size(); ++i)
	{
		auto texResource = m_Textures[mSkinnedTextureNames[i]]->GetResource();
		assert(texResource != nullptr);
		tex2DList.push_back(m_Textures[mSkinnedTextureNames[i]].get());
	}

	auto skyCubeMap = m_Textures["skyCubeMap"]->GetResource();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	//CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(m_SrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(DX12DescriptorAllocator::Allocate(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, (uint32)tex2DList.size() + 1));
	for (uint32 i = 0; i < (uint32)tex2DList.size(); ++i)
	{
		tex2DList[i]->SetTextureId(i);
		srvDesc.Format = tex2DList[i]->GetResource()->GetDesc().Format;
		srvDesc.Texture2D.MipLevels = tex2DList[i]->GetResource()->GetDesc().MipLevels;
		m_Device->CreateShaderResourceView(tex2DList[i]->GetResource().Get(), &srvDesc, hDescriptor);

		// next descriptor
		hDescriptor.Offset(1, DescriptorUtils::GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	}

	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = skyCubeMap->GetDesc().MipLevels;
	srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
	srvDesc.Format = skyCubeMap->GetDesc().Format;
	m_Device->CreateShaderResourceView(skyCubeMap.Get(), &srvDesc, hDescriptor);

	m_Textures["skyCubeMap"]->SetTextureId((UINT)tex2DList.size());
	mShadowMapHeapIndex = m_Textures["skyCubeMap"]->GetTextureId() + 1;
	mSsaoHeapIndexStart = mShadowMapHeapIndex + 1;
	mSsaoAmbientMapIndex = mSsaoHeapIndexStart + 3;
	mNullCubeSrvIndex = mSsaoHeapIndexStart + 5;
	mNullTexSrvIndex1 = mNullCubeSrvIndex + 1;
	mNullTexSrvIndex2 = mNullTexSrvIndex1 + 1;

	auto nullSrv = DX12DescriptorAllocator::CpuOffset(m_SrvDescriptorHeap, mNullCubeSrvIndex);
	mNullSrv = DX12DescriptorAllocator::GpuOffset(m_SrvDescriptorHeap, mNullCubeSrvIndex);

	m_Device->CreateShaderResourceView(nullptr, &srvDesc, nullSrv);
	nullSrv.Offset(1, DescriptorUtils::GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	m_Device->CreateShaderResourceView(nullptr, &srvDesc, nullSrv);

	nullSrv.Offset(1, DescriptorUtils::GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	m_Device->CreateShaderResourceView(nullptr, &srvDesc, nullSrv);

	//m_ShadowMap->BuildDescriptors(GetCpuSrv(mShadowMapHeapIndex), GetGpuSrv(mShadowMapHeapIndex), GetDsv(1));
	m_ShadowMap->BuildDescriptors(DX12DescriptorAllocator::CpuOffset(m_SrvDescriptorHeap, mShadowMapHeapIndex),
		DX12DescriptorAllocator::GpuOffset(m_SrvDescriptorHeap, mShadowMapHeapIndex),
		DX12DescriptorAllocator::CpuOffset(m_DsvHeap, 1));

	m_Ssao->BuildDescriptors(
		m_DepthStencilBuffer.Get(),
		//GetCpuSrv(mSsaoHeapIndexStart),
		DX12DescriptorAllocator::CpuOffset(m_SrvDescriptorHeap, mSsaoHeapIndexStart),
		//GetGpuSrv(mSsaoHeapIndexStart),
		DX12DescriptorAllocator::GpuOffset(m_SrvDescriptorHeap, mSsaoHeapIndexStart),
		DX12DescriptorAllocator::CpuOffset(m_RtvDescriptorHeap, s_SwapChainBufferCount),
		DescriptorUtils::GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), //m_CbvSrvUavDescriptorSize,
		DescriptorUtils::GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));// m_RtvDescriptorSize);

	/*nullSrv.Offset(1, mCbvSrvUavDescriptorSize);
	auto mFontTextureView = mNullTexSrvIndex2 + 1;
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		md3dDevice->CreateShaderResourceView(mTextures["font"]->GetResource().Get(), &srvDesc, nullSrv);
	}*/

}

void DX12RHI::InitBaseRenderItems()
{
	auto skyRitem = std::make_unique<DX12RenderItem>();
	XMStoreFloat4x4((XMFLOAT4X4*)(&skyRitem->World), XMMatrixScaling(5000.0f, 5000.0f, 5000.0f));
	skyRitem->TexTransform = Matrix_4_4::UnitMatrix();//Identity4x4();
	skyRitem->ObjCBIndex = 0;
	skyRitem->Mat = m_Materials["sky"].get();
	skyRitem->Geo = m_Geometries["shapeGeo"].get();
	//skyRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	skyRitem->IndexCount = skyRitem->Geo->DrawArgs["sphere"].IndexCount;
	skyRitem->StartIndexLocation = skyRitem->Geo->DrawArgs["sphere"].StartIndexLocation;
	skyRitem->BaseVertexLocation = skyRitem->Geo->DrawArgs["sphere"].BaseVertexLocation;

	m_RitemLayer[(int)RenderLayer::Sky].push_back(skyRitem.get());
	m_AllRitems.push_back(std::move(skyRitem));

	auto quadRitem = std::make_unique<DX12RenderItem>();
	quadRitem->World = Matrix_4_4::UnitMatrix();// Identity4x4();
	quadRitem->TexTransform = Matrix_4_4::UnitMatrix();//Identity4x4();
	quadRitem->ObjCBIndex = 1;
	quadRitem->Mat = m_Materials["bricks0"].get();
	quadRitem->Geo = m_Geometries["shapeGeo"].get();
	//quadRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	quadRitem->IndexCount = quadRitem->Geo->DrawArgs["quad"].IndexCount;
	quadRitem->StartIndexLocation = quadRitem->Geo->DrawArgs["quad"].StartIndexLocation;
	quadRitem->BaseVertexLocation = quadRitem->Geo->DrawArgs["quad"].BaseVertexLocation;

	//mRitemLayer[(int)RenderLayer::Debug].push_back(quadRitem.get());
	m_AllRitems.push_back(std::move(quadRitem));

	auto boxRitem = std::make_unique<DX12RenderItem>();
	XMStoreFloat4x4((XMFLOAT4X4*)(&boxRitem->World), XMMatrixScaling(2.0f, 1.0f, 2.0f) * XMMatrixTranslation(1.0f, 0.5f, 1.0f));
	XMStoreFloat4x4((XMFLOAT4X4*)(&boxRitem->TexTransform), XMMatrixScaling(1.0f, 1.0f, 1.0f));
	boxRitem->ObjCBIndex = 2;
	boxRitem->Mat = m_Materials["bricks0"].get();
	boxRitem->Geo = m_Geometries["shapeGeo"].get();
	//boxRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	boxRitem->IndexCount = boxRitem->Geo->DrawArgs["box"].IndexCount;
	boxRitem->StartIndexLocation = boxRitem->Geo->DrawArgs["box"].StartIndexLocation;
	boxRitem->BaseVertexLocation = boxRitem->Geo->DrawArgs["box"].BaseVertexLocation;

	m_RitemLayer[(int)RenderLayer::Opaque].push_back(boxRitem.get());
	m_AllRitems.push_back(std::move(boxRitem));

	auto gridRitem = std::make_unique<DX12RenderItem>();
	gridRitem->World = Matrix_4_4::UnitMatrix();//Identity4x4();
	XMStoreFloat4x4((XMFLOAT4X4*)(&gridRitem->TexTransform), XMMatrixScaling(8.0f, 8.0f, 1.0f));
	gridRitem->ObjCBIndex = 3;
	gridRitem->Mat = m_Materials["tile0"].get();
	gridRitem->Geo = m_Geometries["shapeGeo"].get();
	//gridRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gridRitem->IndexCount = gridRitem->Geo->DrawArgs["grid"].IndexCount;
	gridRitem->StartIndexLocation = gridRitem->Geo->DrawArgs["grid"].StartIndexLocation;
	gridRitem->BaseVertexLocation = gridRitem->Geo->DrawArgs["grid"].BaseVertexLocation;

	m_RitemLayer[(int)RenderLayer::Opaque].push_back(gridRitem.get());
	m_AllRitems.push_back(std::move(gridRitem));

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

		XMStoreFloat4x4((XMFLOAT4X4*)(&leftCylRitem->World), rightCylWorld);
		XMStoreFloat4x4((XMFLOAT4X4*)(&leftCylRitem->TexTransform), brickTexTransform);
		leftCylRitem->ObjCBIndex = objCBIndex++;
		leftCylRitem->Mat = m_Materials["bricks0"].get();
		leftCylRitem->Geo = m_Geometries["shapeGeo"].get();
		//leftCylRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		leftCylRitem->IndexCount = leftCylRitem->Geo->DrawArgs["cylinder"].IndexCount;
		leftCylRitem->StartIndexLocation = leftCylRitem->Geo->DrawArgs["cylinder"].StartIndexLocation;
		leftCylRitem->BaseVertexLocation = leftCylRitem->Geo->DrawArgs["cylinder"].BaseVertexLocation;

		XMStoreFloat4x4((XMFLOAT4X4*)(&rightCylRitem->World), leftCylWorld);
		XMStoreFloat4x4((XMFLOAT4X4*)(&rightCylRitem->TexTransform), brickTexTransform);
		rightCylRitem->ObjCBIndex = objCBIndex++;
		rightCylRitem->Mat = m_Materials["bricks0"].get();
		rightCylRitem->Geo = m_Geometries["shapeGeo"].get();
		//rightCylRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		rightCylRitem->IndexCount = rightCylRitem->Geo->DrawArgs["cylinder"].IndexCount;
		rightCylRitem->StartIndexLocation = rightCylRitem->Geo->DrawArgs["cylinder"].StartIndexLocation;
		rightCylRitem->BaseVertexLocation = rightCylRitem->Geo->DrawArgs["cylinder"].BaseVertexLocation;

		XMStoreFloat4x4((XMFLOAT4X4*)(&leftSphereRitem->World), leftSphereWorld);
		leftSphereRitem->TexTransform = Matrix_4_4::UnitMatrix();//Identity4x4();
		leftSphereRitem->ObjCBIndex = objCBIndex++;
		leftSphereRitem->Mat = m_Materials["mirror0"].get();
		leftSphereRitem->Geo = m_Geometries["shapeGeo"].get();
		//leftSphereRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		leftSphereRitem->IndexCount = leftSphereRitem->Geo->DrawArgs["sphere"].IndexCount;
		leftSphereRitem->StartIndexLocation = leftSphereRitem->Geo->DrawArgs["sphere"].StartIndexLocation;
		leftSphereRitem->BaseVertexLocation = leftSphereRitem->Geo->DrawArgs["sphere"].BaseVertexLocation;

		XMStoreFloat4x4((XMFLOAT4X4*)(&rightSphereRitem->World), rightSphereWorld);
		rightSphereRitem->TexTransform = Matrix_4_4::UnitMatrix();//Identity4x4();
		rightSphereRitem->ObjCBIndex = objCBIndex++;
		rightSphereRitem->Mat = m_Materials["mirror0"].get();
		rightSphereRitem->Geo = m_Geometries["shapeGeo"].get();
		//rightSphereRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		rightSphereRitem->IndexCount = rightSphereRitem->Geo->DrawArgs["sphere"].IndexCount;
		rightSphereRitem->StartIndexLocation = rightSphereRitem->Geo->DrawArgs["sphere"].StartIndexLocation;
		rightSphereRitem->BaseVertexLocation = rightSphereRitem->Geo->DrawArgs["sphere"].BaseVertexLocation;

		m_RitemLayer[(int)RenderLayer::Opaque].push_back(leftCylRitem.get());
		m_RitemLayer[(int)RenderLayer::Opaque].push_back(rightCylRitem.get());
		m_RitemLayer[(int)RenderLayer::Opaque].push_back(leftSphereRitem.get());
		m_RitemLayer[(int)RenderLayer::Opaque].push_back(rightSphereRitem.get());

		m_AllRitems.push_back(std::move(leftCylRitem));
		m_AllRitems.push_back(std::move(rightCylRitem));
		m_AllRitems.push_back(std::move(leftSphereRitem));
		m_AllRitems.push_back(std::move(rightSphereRitem));
	}

	for (UINT i = 0; i < mSkinnedMats.size(); ++i)
	{
		std::string submeshName = "sm_" + std::to_string(i);

		auto ritem = std::make_unique<DX12RenderItem>();

		// Reflect to change coordinate system from the RHS the data was exported out as.
		XMMATRIX modelScale = XMMatrixScaling(0.05f, 0.05f, -0.05f);
		XMMATRIX modelRot = XMMatrixRotationY(PI);
		XMMATRIX modelOffset = XMMatrixTranslation(0.0f, 0.0f, -5.0f);
		XMStoreFloat4x4((XMFLOAT4X4*)(&ritem->World), modelScale * modelRot * modelOffset);

		ritem->TexTransform = Matrix_4_4::UnitMatrix();//Identity4x4();
		ritem->ObjCBIndex = objCBIndex++;
		ritem->Mat = m_Materials[mSkinnedMats[i].m_Name].get();
		ritem->Geo = m_Geometries["soldier"].get();
		//ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		ritem->IndexCount = ritem->Geo->DrawArgs[submeshName].IndexCount;
		ritem->StartIndexLocation = ritem->Geo->DrawArgs[submeshName].StartIndexLocation;
		ritem->BaseVertexLocation = ritem->Geo->DrawArgs[submeshName].BaseVertexLocation;

		// All render items for this solider.m3d instance share
		// the same skinned model instance.
		ritem->SkinnedCBIndex = 0;
		ritem->SkinnedModelInst = mSkinnedModelInst.get();

		m_RitemLayer[(int)RenderLayer::SkinnedOpaque].push_back(ritem.get());
		m_AllRitems.push_back(std::move(ritem));
	}

	{
		/*auto uiQuadRitem = std::make_unique<DX12RenderItem>();
		XMStoreFloat4x4(&uiQuadRitem->World, XMMatrixScaling(5.0f, 5.0f, 2.0f) * XMMatrixTranslation(0.0f, 0.5f, 0.0f));
		XMStoreFloat4x4(&uiQuadRitem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
		uiQuadRitem->ObjCBIndex = objCBIndex++;
		uiQuadRitem->Mat = mMaterials["mirror0"].get();
		uiQuadRitem->Geo = mGeometries["shapeGeo"].get();
		uiQuadRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		uiQuadRitem->IndexCount = uiQuadRitem->Geo->DrawArgs["box"].IndexCount;
		uiQuadRitem->StartIndexLocation = uiQuadRitem->Geo->DrawArgs["box"].StartIndexLocation;
		uiQuadRitem->BaseVertexLocation = uiQuadRitem->Geo->DrawArgs["box"].BaseVertexLocation;

		mRitemLayer[(int)RenderLayer::Opaque].push_back(uiQuadRitem.get());
		mAllRitems.push_back(std::move(uiQuadRitem));*/

		auto quadRitem = std::make_unique<DX12RenderItem>();
		//XMStoreFloat4x4((XMFLOAT4X4*)(&quadRitem->World), XMMatrixScaling(2.f, 2.f, 1.0f)* XMMatrixTranslation(-0.0f, 0.0f, 0.0f));
		XMStoreFloat4x4((XMFLOAT4X4*)(&quadRitem->World), XMMatrixScaling(2.f, 2.f, 1.0f)* XMMatrixTranslation(-1.0f, 1.0f, 0.0f));
		quadRitem->TexTransform = Matrix_4_4::UnitMatrix();//Identity4x4();
		quadRitem->ObjCBIndex = objCBIndex++;
		quadRitem->Mat = m_Materials["bricks0"].get();
		quadRitem->Geo = m_Geometries["ui"].get();
		//quadRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		quadRitem->IndexCount = quadRitem->Geo->m_IndexBuffer->GetIndexCount();//quadRitem->Geo->DrawArgs["ui"].IndexCount;
		quadRitem->StartIndexLocation = 0;//quadRitem->Geo->DrawArgs["ui"].StartIndexLocation;
		quadRitem->BaseVertexLocation = 0;//quadRitem->Geo->DrawArgs["quad"].BaseVertexLocation;

		m_UIRenderItem = quadRitem.get();
		m_RitemLayer[(int)RenderLayer::Debug].push_back(quadRitem.get());
		m_AllRitems.push_back(std::move(quadRitem));
	}
}

void DX12RHI::InitFrameBuffer()
{
	for (int i = 0; i < s_FrameBufferCount; ++i)
	{
		m_FrameBuffer.push_back(MakeUnique<DX12FrameBuffer>(m_Device.Get(), 2, m_MaxRenderItemCount/*(UINT)m_AllRitems.size()*/,
			1, (int)m_Materials.size()));
	}
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
