#include "RiverPch.h"
#include "Renderer/DX12Renderer/Header/DX12Shader.h"
#include "Renderer/DX12Renderer/Header/DX12Util.h"
#include <d3dcompiler.h>

#include "Utils/Header/StringUtils.h"

DX12Shader::DX12Shader(ID3D12Device* device, const String& filePath, Pair<const D3D_SHADER_MACRO*, const D3D_SHADER_MACRO*>& defines,
	Pair<const char*, const char*>& name, Pair<const char*, const char*> target, V_Array<D3D12_INPUT_ELEMENT_DESC>* layout,
	ShaderParam* param)
{
#if defined(_DEBUG)
	//调试状态下，打开Shader编译的调试标志，不优化
	UINT nCompileFlags = /*D3DCOMPILE_ENABLE_STRICTNESS |*/
		D3DCOMPILE_DEBUG
		| D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT nCompileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#endif

	auto path = S_2_WS(filePath);
	if (name.first)
	{
		Microsoft::WRL::ComPtr<ID3DBlob> errors;
		ThrowIfFailed(D3DCompileFromFile(path.c_str(), defines.first, D3D_COMPILE_STANDARD_FILE_INCLUDE, name.first, target.first,
			nCompileFlags, 0, &m_VSByteCode, &errors));

		if (errors != nullptr)
		{
			OutputDebugStringA((char*)errors->GetBufferPointer());
		}
	}

	if (name.second)
	{
		Microsoft::WRL::ComPtr<ID3DBlob> errors1;
		ThrowIfFailed(D3DCompileFromFile(path.c_str(), defines.second, D3D_COMPILE_STANDARD_FILE_INCLUDE, name.second, target.second, 
			nCompileFlags, 0, &m_PSByteCode, &errors1));

		if (errors1 != nullptr)
		{
			OutputDebugStringA((char*)errors1->GetBufferPointer());
		}
	}

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

		Microsoft::WRL::ComPtr<ID3DBlob> pISignatureBlob;
		Microsoft::WRL::ComPtr<ID3DBlob> pIErrorBlob;

		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pISignatureBlob, &pIErrorBlob);

		if (pIErrorBlob != nullptr)
		{
			::OutputDebugStringA((char*)pIErrorBlob->GetBufferPointer());
		}
		ThrowIfFailed(hr);

		ThrowIfFailed(device->CreateRootSignature(0, pISignatureBlob->GetBufferPointer(), pISignatureBlob->GetBufferSize()
			, IID_PPV_ARGS(&m_RootSignature)));
	}

	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;

		ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		psoDesc.pRootSignature = m_RootSignature.Get();
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = false ? 4 : 1;
		psoDesc.SampleDesc.Quality = false ? (0 - 1) : 0;
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		if (param)
		{
			psoDesc.RasterizerState.CullMode = (D3D12_CULL_MODE)param->Cull;
			psoDesc.DepthStencilState.DepthFunc = (D3D12_COMPARISON_FUNC)param->DepthComparisonFunc;

			if (param->DefaultType == ShaderDefaultType::ShadowMap)
			{
				psoDesc.RasterizerState.DepthBias = 100000;
				psoDesc.RasterizerState.DepthBiasClamp = 0.f;
				psoDesc.RasterizerState.SlopeScaledDepthBias = 1.f;
				psoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
				psoDesc.NumRenderTargets = 0;
			}
		}

		psoDesc.InputLayout = { layout->data(), (UINT)layout->size() };

		psoDesc.VS = { reinterpret_cast<BYTE*>(m_VSByteCode->GetBufferPointer()), m_VSByteCode->GetBufferSize() };
		psoDesc.PS = { reinterpret_cast<BYTE*>(m_PSByteCode->GetBufferPointer()), m_PSByteCode->GetBufferSize() };

		ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PipelineState)));
	}
}

DX12Shader::~DX12Shader()
{
}