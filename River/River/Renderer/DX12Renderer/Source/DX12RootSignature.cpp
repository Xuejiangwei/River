#include "RiverPch.h"
#include "Renderer/DX12Renderer/Header/DX12RootSignature.h"
#include "Renderer/DX12Renderer/Header/DX12Util.h"

DX12RootSignature::DX12RootSignature(ID3D12Device* device, CD3DX12_ROOT_SIGNATURE_DESC& desc)
{
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

DX12RootSignature::~DX12RootSignature()
{
}