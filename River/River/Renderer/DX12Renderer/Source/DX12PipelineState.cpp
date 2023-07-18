#include "RiverPch.h"
#include "Renderer/DX12Renderer/Header/DX12Util.h"
#include "Renderer/DX12Renderer/Header/d3dx12.h"
#include "Renderer/DX12Renderer/Header/DX12PipelineState.h"
#include "Renderer/DX12Renderer/Header/DX12Shader.h"

DX12PipelineState::DX12PipelineState(ID3D12Device* device, Share<Shader> shader) 
	:m_Shader(shader)
{
	InitRootSignature(device);
	InitPipelineState(device);
}

DX12PipelineState::~DX12PipelineState()
{
}

void DX12PipelineState::InitRootSignature(ID3D12Device* device)
{
	D3D12_ROOT_SIGNATURE_DESC stRootSignatureDesc =
	{
		0
		, nullptr
		, 0
		, nullptr
		, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	};

	Microsoft::WRL::ComPtr<ID3DBlob> pISignatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> pIErrorBlob;

	ThrowIfFailed(D3D12SerializeRootSignature(
		&stRootSignatureDesc
		, D3D_ROOT_SIGNATURE_VERSION_1
		, &pISignatureBlob
		, &pIErrorBlob));

	ThrowIfFailed(device->CreateRootSignature(0
		, pISignatureBlob->GetBufferPointer()
		, pISignatureBlob->GetBufferSize()
		, IID_PPV_ARGS(&m_RootSignature)));
}

void DX12PipelineState::InitPipelineState(ID3D12Device* device)
{
	D3D12_INPUT_ELEMENT_DESC stInputElementDescs[] =
	{
		{
			"POSITION"
			, 0
			, DXGI_FORMAT_R32G32B32A32_FLOAT
			, 0
			, 0
			, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA
			, 0
		},
		{
			"COLOR"
			, 0
			, DXGI_FORMAT_R32G32B32A32_FLOAT
			, 0
			, 16
			, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA
			, 0
		}
	};

	auto Shader = dynamic_cast<DX12Shader*>(m_Shader.get());
	DXGI_FORMAT	emRenderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC stPSODesc = {};
	stPSODesc.InputLayout = { stInputElementDescs, _countof(stInputElementDescs) };
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
