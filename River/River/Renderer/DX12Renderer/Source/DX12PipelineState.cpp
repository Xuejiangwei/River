#include "RiverPch.h"
#include "Renderer/DX12Renderer/Header/DX12Util.h"
#include "Renderer/DX12Renderer/Header/d3dx12.h"
#include "Renderer/DX12Renderer/Header/DX12VertexBuffer.h"
#include "Renderer/DX12Renderer/Header/DX12PipelineState.h"
#include "Renderer/DX12Renderer/Header/DX12Shader.h"

DX12PipelineState::DX12PipelineState(ID3D12Device* device, Share<Shader> shader, Share<VertexBuffer> vertexBuffer)
	:m_Shader(shader)
{
	InitRootSignature(device);
	InitPipelineState(device, vertexBuffer);
}

DX12PipelineState::~DX12PipelineState()
{
}

void DX12PipelineState::InitRootSignature(ID3D12Device* device)
{
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

	CD3DX12_ROOT_SIGNATURE_DESC stRootSignatureDesc(1, slotRootParameter, 0, nullptr
		, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	Microsoft::WRL::ComPtr<ID3DBlob> pISignatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> pIErrorBlob;

	HRESULT hr = D3D12SerializeRootSignature(
		&stRootSignatureDesc
		, D3D_ROOT_SIGNATURE_VERSION_1
		, &pISignatureBlob
		, &pIErrorBlob);

	if (pIErrorBlob!= nullptr)
	{
		::OutputDebugStringA((char*)pIErrorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);


	ThrowIfFailed(device->CreateRootSignature(0, pISignatureBlob->GetBufferPointer(), pISignatureBlob->GetBufferSize()
		, IID_PPV_ARGS(&m_RootSignature)));
}

void DX12PipelineState::InitPipelineState(ID3D12Device* device, Share<VertexBuffer> vertexBuffer)
{
	auto Shader = dynamic_cast<DX12Shader*>(m_Shader.get());
	auto dx12VertexBuffer = dynamic_cast<DX12VertexBuffer*>(vertexBuffer.get());
	DXGI_FORMAT	emRenderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC stPSODesc = {};
	stPSODesc.InputLayout = { dx12VertexBuffer->m_VertexLayout.data(), (uint32_t)dx12VertexBuffer->m_VertexLayout.size() };
	stPSODesc.pRootSignature = m_RootSignature.Get();
	stPSODesc.VS.pShaderBytecode = Shader->m_VertexShaderByteCode->GetBufferPointer();
	stPSODesc.VS.BytecodeLength = Shader->m_VertexShaderByteCode->GetBufferSize();
	stPSODesc.PS.pShaderBytecode = Shader->m_PixelShaderByteCode->GetBufferPointer();
	stPSODesc.PS.BytecodeLength = Shader->m_PixelShaderByteCode->GetBufferSize();

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
