#include "RiverPch.h"
#include "Renderer/DX12Renderer/Header/DX12Shader.h"
#include "Renderer/DX12Renderer/Header/DX12Util.h"
#include <d3dcompiler.h>

#include "Utils/Header/StringUtils.h"

DX12Shader::DX12Shader(ID3D12Device* device, const String& filePath, Pair<const D3D_SHADER_MACRO*, const D3D_SHADER_MACRO*>& defines,
	Pair<const char*, const char*>& name, Pair<const char*, const char*> target, V_Array<D3D12_INPUT_ELEMENT_DESC>* layout)
{
#if defined(_DEBUG)
	//调试状态下，打开Shader编译的调试标志，不优化
	UINT nCompileFlags = D3DCOMPILE_ENABLE_STRICTNESS |
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

	if (true)
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
	else
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

		Microsoft::WRL::ComPtr<ID3DBlob> pISignatureBlob;
		Microsoft::WRL::ComPtr<ID3DBlob> pIErrorBlob;

		HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &pISignatureBlob, &pIErrorBlob);

		if (pIErrorBlob != nullptr)
		{
			::OutputDebugStringA((char*)pIErrorBlob->GetBufferPointer());
		}
		ThrowIfFailed(hr);

		ThrowIfFailed(device->CreateRootSignature(0, pISignatureBlob->GetBufferPointer(), pISignatureBlob->GetBufferSize()
			, IID_PPV_ARGS(&m_RootSignature)));
	}

	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

		ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		opaquePsoDesc.pRootSignature = m_RootSignature.Get();
		opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		opaquePsoDesc.SampleMask = UINT_MAX;
		opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		opaquePsoDesc.NumRenderTargets = 1;
		opaquePsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		opaquePsoDesc.SampleDesc.Count = false ? 4 : 1;
		opaquePsoDesc.SampleDesc.Quality = false ? (0 - 1) : 0;
		opaquePsoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		opaquePsoDesc.InputLayout = { layout->data(), (UINT)layout->size() };

		opaquePsoDesc.VS = { reinterpret_cast<BYTE*>(m_VSByteCode->GetBufferPointer()), m_VSByteCode->GetBufferSize() };
		opaquePsoDesc.PS = { reinterpret_cast<BYTE*>(m_PSByteCode->GetBufferPointer()), m_PSByteCode->GetBufferSize() };

		ThrowIfFailed(device->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&m_PipelineState)));
	}
}

DX12Shader::~DX12Shader()
{
}