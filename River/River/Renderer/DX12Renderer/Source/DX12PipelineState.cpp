#include "RiverPch.h"
#include "Renderer/DX12Renderer/Header/DX12PipelineState.h"

DX12PipelineState::DX12PipelineState(ID3D12Device* Device, Share<Shader> Shader, const Vector<ShaderLayout>& Layout)
{
	Vector<D3D12_INPUT_ELEMENT_DESC> InputElementDescs(Layout.size());
	for (size_t i = 0; i < InputElementDescs.size(); i++)
	{
		InputElementDescs[i].SemanticName = Layout[i].Name.c_str();
		InputElementDescs[i].SemanticIndex = Layout[i].Index;
		InputElementDescs[i].Format = (DXGI_FORMAT)Layout[i].Format;
		InputElementDescs[i].InputSlot = Layout[i].Slot;
		InputElementDescs[i].AlignedByteOffset = Layout[i].OffsetByte;
		InputElementDescs[i].InputSlotClass = (D3D12_INPUT_CLASSIFICATION)Layout[i].Classification;
		InputElementDescs[i].InstanceDataStepRate = 0;
	}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC stPSODesc = {};
	stPSODesc.InputLayout = { InputElementDescs.begin()._Unwrapped(), (uint32_t)Layout.size()};
	stPSODesc.pRootSignature = m_RootSignature.Get();
	stPSODesc.VS.pShaderBytecode = ((ID3D10Blob*)Shader->GetVertexShader())->GetBufferPointer();
	stPSODesc.VS.BytecodeLength = ((ID3D10Blob*)Shader->GetVertexShader())->GetBufferSize();
	stPSODesc.PS.pShaderBytecode = ((ID3D10Blob*)Shader->GetPixelShader())->GetBufferPointer();
	stPSODesc.PS.BytecodeLength = ((ID3D10Blob*)Shader->GetPixelShader())->GetBufferSize();

	stPSODesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	stPSODesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;

	stPSODesc.BlendState.AlphaToCoverageEnable = FALSE;
	stPSODesc.BlendState.IndependentBlendEnable = FALSE;
	stPSODesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	stPSODesc.DepthStencilState.DepthEnable = FALSE;
	stPSODesc.DepthStencilState.StencilEnable = FALSE;

	stPSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	stPSODesc.NumRenderTargets = 1;
	stPSODesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	stPSODesc.SampleMask = UINT_MAX;
	stPSODesc.SampleDesc.Count = 1;

	Device->CreateGraphicsPipelineState(&stPSODesc, IID_PPV_ARGS(&m_PipelineState));
}

DX12PipelineState::~DX12PipelineState()
{
}