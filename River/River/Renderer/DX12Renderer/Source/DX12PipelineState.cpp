#include "RiverPch.h"
#include "Renderer/DX12Renderer/Header/DX12Util.h"
#include "Renderer/DX12Renderer/Header/d3dx12.h"
#include "Renderer/DX12Renderer/Header/DX12VertexBuffer.h"
#include "Renderer/DX12Renderer/Header/DX12PipelineState.h"
#include "Renderer/DX12Renderer/Header/DX12Shader.h"
#include "Renderer/DX12Renderer/Header/DX12RootSignature.h"

DX12PipelineState::DX12PipelineState(ID3D12Device* device, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc)
{
	ThrowIfFailed(device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&m_PipelineState)));
}

DX12PipelineState::~DX12PipelineState()
{
}

//void DX12PipelineState::InitRootSignature(ID3D12Device* device)
//{
//	CD3DX12_ROOT_PARAMETER slotRootParameter[4];
//	CD3DX12_DESCRIPTOR_RANGE cbvTable;
//	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
//	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);
//
//	CD3DX12_DESCRIPTOR_RANGE cbvTable1;
//	cbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
//	slotRootParameter[1].InitAsDescriptorTable(1, &cbvTable1);
//
//	CD3DX12_DESCRIPTOR_RANGE cbvTable2;
//	cbvTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);
//	slotRootParameter[2].InitAsDescriptorTable(1, &cbvTable2);
//
//	CD3DX12_DESCRIPTOR_RANGE texTable;
//	texTable.Init(
//		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
//		1,  // number of descriptors
//		0); // register t0
//	slotRootParameter[3].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
//	
//	auto staticSamplers = GetStaticSamplers();
//
//	CD3DX12_ROOT_SIGNATURE_DESC stRootSignatureDesc(4, slotRootParameter, (UINT)staticSamplers.size(), staticSamplers.data()
//		, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
//
//	Microsoft::WRL::ComPtr<ID3DBlob> pISignatureBlob;
//	Microsoft::WRL::ComPtr<ID3DBlob> pIErrorBlob;
//
//	HRESULT hr = D3D12SerializeRootSignature(
//		&stRootSignatureDesc
//		, D3D_ROOT_SIGNATURE_VERSION_1
//		, &pISignatureBlob
//		, &pIErrorBlob);
//
//	if (pIErrorBlob != nullptr)
//	{
//		::OutputDebugStringA((char*)pIErrorBlob->GetBufferPointer());
//	}
//	ThrowIfFailed(hr);
//
//	ThrowIfFailed(device->CreateRootSignature(0, pISignatureBlob->GetBufferPointer(), pISignatureBlob->GetBufferSize()
//		, IID_PPV_ARGS(&m_RootSignature)));
//}