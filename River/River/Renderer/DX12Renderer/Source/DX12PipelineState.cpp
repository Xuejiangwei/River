#include "RiverPch.h"
#include "Renderer/DX12Renderer/Header/DX12Util.h"
#include "Renderer/DX12Renderer/Header/d3dx12.h"
#include "Renderer/DX12Renderer/Header/DX12VertexBuffer.h"
#include "Renderer/DX12Renderer/Header/DX12PipelineState.h"
#include "Renderer/DX12Renderer/Header/DX12Shader.h"
#include "Renderer/DX12Renderer/Header/DX12RootSignature.h"

DX12PipelineState::DX12PipelineState(ID3D12Device* device, Share<DX12RootSignature> rootSignature, Share<Shader> vsShader, Share<Shader> psShader, Share<VertexBuffer> vertexBuffer)
{
	auto dx12VertexBuffer = dynamic_cast<DX12VertexBuffer*>(vertexBuffer.get());
	DXGI_FORMAT	emRenderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC stPSODesc = {};
	stPSODesc.InputLayout = { dx12VertexBuffer->m_VertexLayout.data(), (uint32_t)dx12VertexBuffer->m_VertexLayout.size() };
	stPSODesc.pRootSignature = rootSignature->GetRootSignature();

	if (vsShader)
	{
		auto vs_Shader = dynamic_cast<DX12Shader*>(vsShader.get());
		stPSODesc.VS.pShaderBytecode = vs_Shader->m_ShaderByteCode->GetBufferPointer();
		stPSODesc.VS.BytecodeLength = vs_Shader->m_ShaderByteCode->GetBufferSize();
	}

	if (psShader)
	{
		auto ps_Shader = dynamic_cast<DX12Shader*>(psShader.get());
		stPSODesc.PS.pShaderBytecode = ps_Shader->m_ShaderByteCode->GetBufferPointer();
		stPSODesc.PS.BytecodeLength = ps_Shader->m_ShaderByteCode->GetBufferSize();
	}

	stPSODesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	stPSODesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;

	stPSODesc.BlendState.AlphaToCoverageEnable = FALSE;
	stPSODesc.BlendState.IndependentBlendEnable = FALSE;
	stPSODesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	stPSODesc.DepthStencilState.DepthEnable = FALSE;
	stPSODesc.DepthStencilState.StencilEnable = FALSE;

	stPSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	stPSODesc.NumRenderTargets = 1;
	stPSODesc.RTVFormats[0] = emRenderTargetFormat;

	stPSODesc.SampleMask = UINT_MAX;
	stPSODesc.SampleDesc.Count = 1;

	ThrowIfFailed(device->CreateGraphicsPipelineState(&stPSODesc, IID_PPV_ARGS(&m_PipelineState)));
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