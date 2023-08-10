#include "RiverPch.h"
#include "Renderer/DX12Renderer/Header/DX12ShadowPass.h"
#include "Renderer/DX12Renderer/Header/DX12FrameBuffer.h"

//float mLightNearZ = 0.0f;
//float mLightFarZ = 0.0f;
//DirectX::XMFLOAT3 mLightPosW;
//DirectX::XMFLOAT4X4 mLightView = Identity4x4();
//DirectX::XMFLOAT4X4 mLightProj = Identity4x4();
//DirectX::XMFLOAT4X4 mShadowTransform = Identity4x4();

DX12ShadowPass::DX12ShadowPass(ID3D12Device* device, UINT width, UINT height)
	: m_Device(device), m_Width(width), m_Height(height)
{
	m_Viewport = { 0.0f,0.0f, (float)m_Width, (float)m_Height, 0.0f, 1.0f };
	m_ScissorRect = { 0, 0, (int)m_Width, (int)m_Height };

	BuildResource();
}

DX12ShadowPass::~DX12ShadowPass()
{
}

void DX12ShadowPass::Initialize()
{
}

void DX12ShadowPass::Draw()
{
}

void DX12ShadowPass::Update(FrameBuffer* frameBuffer)
{
	/*using namespace DirectX;
	static PassUniform mShadowPassCB;

	auto currFrameBuffer = dynamic_cast<DX12FrameBuffer*>(frameBuffer);
	
	XMMATRIX view = XMLoadFloat4x4(&mLightView);
	XMMATRIX proj = XMLoadFloat4x4(&mLightProj);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&mShadowPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mShadowPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mShadowPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mShadowPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mShadowPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mShadowPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	mShadowPassCB.EyePosW = mLightPosW;
	mShadowPassCB.RenderTargetSize = XMFLOAT2((float)m_Width, (float)m_Height);
	mShadowPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / m_Width, 1.0f / m_Height);
	mShadowPassCB.NearZ = mLightNearZ;
	mShadowPassCB.FarZ = mLightFarZ;

	auto currPassCB = currFrameBuffer->m_PassUniform.get();
	currPassCB->CopyData(1, mShadowPassCB);*/
}

void DX12ShadowPass::OnResize(int width, int height)
{
	if (m_Width != width || m_Height != height)
	{
		m_Width = width;
		m_Height = height;

		BuildResource();

		// New resource, so we need new descriptors to that resource.
		BuildSrvAndDsv();
	}
}

void DX12ShadowPass::BuildResource()
{
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = m_Width;
	texDesc.Height = m_Height;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = m_Format;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	auto hProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(m_Device->CreateCommittedResource(&hProp, D3D12_HEAP_FLAG_NONE, &texDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
		&optClear, IID_PPV_ARGS(&m_ShadowMap)));
}

void DX12ShadowPass::BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuSrv, CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuSrv, CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDsv)
{
	m_HandleCpuSrv = hCpuSrv;
	m_HandleGpuSrv = hGpuSrv;
	m_HandleCpuDsv = hCpuDsv;

	BuildSrvAndDsv();
}

void DX12ShadowPass::BuildSrvAndDsv()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.PlaneSlice = 0;
	m_Device->CreateShaderResourceView(m_ShadowMap.Get(), &srvDesc, m_HandleCpuSrv);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.Texture2D.MipSlice = 0;
	m_Device->CreateDepthStencilView(m_ShadowMap.Get(), &dsvDesc, m_HandleCpuDsv);
}
