#include "RiverPch.h"
#include "Renderer/Pass/Header/RenderPassForwardRendering.h"
#include "Renderer/Pass/Header/RenderPassShadow.h"
#include "Renderer/Header/RHI.h"
#include "Renderer/Header/RenderScene.h"
#include "Renderer/Header/RenderProxy.h"
#include "Object/Header/LightObject.h"
#include "Object/Header/CameraObject.h"
#include "Application.h"
#include "Math/Header/Geometric.h"

RenderPassForwardRendering::RenderPassForwardRendering()
{
	m_CommandId = RHI::Get()->AllocDrawCommand();
}

RenderPassForwardRendering::~RenderPassForwardRendering()
{
}

void RenderPassForwardRendering::Render()
{
	auto& rhi = RHI::Get();
	auto renderScene = Application::Get()->GetRenderScene();
	auto [w, h] = Application::Get()->GetWindow()->GetWindowSize();

	rhi->SetViewPort(w, h);

	//获得光源
	auto& lightProxys = renderScene->GetRenderLightProxys();
	for (size_t i = 0; i < lightProxys.size() && i < _countof(m_PassUniform.Lights); i++)
	{
		auto lightObject = dynamic_cast<LightObject*>(lightProxys[i]->GetObject());
		m_PassUniform.Lights[i].Direction = lightObject->GetDirection();
		m_PassUniform.Lights[i].Strength = lightObject->GetLightStrength();
	}
	
	//获得相机
	auto cameraProxys = renderScene->GetRenderCameraProxys();
	if (cameraProxys.size() > 0)
	{
		auto camera = dynamic_cast<CameraObject*>(cameraProxys[0]->GetObject());
		
		auto view = camera->GetViewMatrix();
		auto proj = camera->GetProjectMatrix();

		auto viewProj = Matrix4x4::Multiply(view, proj);
		auto invView = Matrix4x4_Inverse(&Matrix4x4_Determinant(view), view);
		auto invProj = Matrix4x4_Inverse(&Matrix4x4_Determinant(proj), proj);
		auto invViewProj = Matrix4x4_Inverse(&Matrix4x4_Determinant(viewProj), viewProj);

		// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
		Matrix4x4 T(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f);

		auto viewProjTex = Matrix4x4::Multiply(viewProj, T);

		m_PassUniform.View = Matrix4x4_Transpose(*(Matrix4x4*)&view);
		m_PassUniform.InvView = Matrix4x4_Transpose(*(Matrix4x4*)&invView);
		m_PassUniform.Proj = Matrix4x4_Transpose(*(Matrix4x4*)&proj);
		m_PassUniform.InvProj = Matrix4x4_Transpose(*(Matrix4x4*)&invProj);
		m_PassUniform.ViewProj = Matrix4x4_Transpose(*(Matrix4x4*)&viewProj);
		m_PassUniform.InvViewProj = Matrix4x4_Transpose(*(Matrix4x4*)&invViewProj);
		m_PassUniform.ViewProjTex = Matrix4x4_Transpose(*(Matrix4x4*)&viewProjTex);

		auto shadowPass = renderScene->GetRenderPassByType(RenderPassType::Shadow);
		if (shadowPass)
		{
			Matrix4x4 shadowTransform = dynamic_cast<RenderPassShadow*>(shadowPass)->GetShadowTransform();
			m_PassUniform.ShadowTransform = Matrix4x4_Transpose(*(Matrix4x4*)&shadowTransform);
		}

		m_PassUniform.EyePosW = camera->GetPosition();
		m_PassUniform.RenderTargetSize = { (float)w, (float)h };
		m_PassUniform.InvRenderTargetSize = { 1.0f / w, 1.0f / h };
		m_PassUniform.NearZ = 1.0f;
		m_PassUniform.FarZ = 1000.0f;
		m_PassUniform.TotalTime = 0;//time.TotalTime();
		m_PassUniform.DeltaTime = 0;//time.DeltaTime();
		m_PassUniform.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	
		rhi->UpdatePassUniform(0, &m_PassUniform);
	}


	//先渲染非透明队列
	auto& renderProxys = renderScene->GetRenderProxys(MaterialBlendMode::Opaque);
	for (auto& proxy : renderProxys)
	{
		auto renderProxy = static_cast<RenderProxy*>(proxy);
		if (renderProxy)
		{	
			for (auto& id : renderProxy->GetRenderItemIds())
			{
				auto renderItem = rhi->GetRenderItem(id);
				m_RenderBatch[renderItem->Material ? renderItem->Material->m_Shader->GetShaderId() : 0].push_back(id);
			}
		}
	}

	for (auto& batch : m_RenderBatch)
	{
		for (auto& renderItemId : batch.second)
		{
			rhi->DrawRenderItem(renderItemId);
		}
	}

	rhi->DrawRenderPass(this, FrameBufferType::Color);
	//rhi->Render();

	m_RenderBatch.clear();
}
