#include "RiverPch.h"
#include "Renderer/Pass/Header/RenderPassShadow.h"
#include "Application.h"
#include "Math/Header/Geometric.h"
#include "Renderer/Header/RHI.h"
#include "Renderer/Header/RenderScene.h"
#include "Renderer/Header/RenderProxy.h"
#include "Object/Header/LightObject.h"
#include "Renderer/Header/AssetManager.h"

RenderPassShadow::RenderPassShadow()
{
	m_CommandId = RHI::Get()->AllocDrawCommand();
	m_ShadowMapTexture = AssetManager::Get()->GetTexture("ShadowMap"); //Texture::CreateTexture("ShadowMap", m_ShadowMapSize.x, m_ShadowMapSize.y);
	m_ShadowMapSize = { (float)m_ShadowMapTexture->GetTextureWidth(), (float)m_ShadowMapTexture->GetTextureHeight() };
}

RenderPassShadow::~RenderPassShadow()
{
}

void RenderPassShadow::Render()
{
	/*if (true)
	{
		return;
	}*/

	auto& rhi = RHI::Get();
	auto renderScene = Application::Get()->GetRenderScene();

	rhi->SetViewPort(720, 720);

	LightObject* mainLight = nullptr;
	auto& lightProxys = renderScene->GetRenderLightProxys();
	int index = 0;
	for (auto& proxy : lightProxys)
	{
		auto light = dynamic_cast<LightObject*>(proxy->GetObject());
		if (!mainLight)
		{
			mainLight = light;
		}

		m_PassUniform.Lights[index].Direction = light->GetDirection();
		m_PassUniform.Lights[index].Strength = light->GetComponent<LightComponent>()->GetLightStrength();
		index++;
	}

	if (mainLight)
	{
		float radius = 18.f;
		auto lightDir = mainLight->GetDirection();
		auto lightPos = -2.0f * radius * lightDir;
		auto targetPos = Float3();
		auto lightUp = Float3(0, 1, 0);
		auto lightView = Matrix4x4_LookAtLH(lightPos, targetPos, lightUp);
		auto sphereCenterLS = Vector3TransformCoord(GetFloat3(targetPos), lightView);

		float l = sphereCenterLS.x - radius;
		float b = sphereCenterLS.y - radius;
		float n = sphereCenterLS.z - radius;
		float r = sphereCenterLS.x + radius;
		float t = sphereCenterLS.y + radius;
		float f = sphereCenterLS.z + radius;

		float mLightNearZ = n;
		float mLightFarZ = f;
		auto lightProj = Matrix4x4_OrthographicOffCenterLH(l, r, b, t, n, f);

		Matrix4x4 T(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f);

		Matrix4x4 shadowTransform = lightView * lightProj * T;
		auto viewProj = lightView * lightProj;

		m_PassUniform.View = Matrix4x4_Transpose(lightView);
		m_PassUniform.InvView = Matrix4x4_Transpose(Matrix4x4_Inverse(&Matrix4x4_Determinant(lightView), lightView));
		m_PassUniform.Proj = Matrix4x4_Transpose(lightProj);
		m_PassUniform.InvProj = Matrix4x4_Transpose(Matrix4x4_Inverse(&Matrix4x4_Determinant(lightProj), lightProj));
		m_PassUniform.ViewProj = Matrix4x4_Transpose(viewProj);
		m_PassUniform.InvViewProj = Matrix4x4_Transpose(Matrix4x4_Inverse(&Matrix4x4_Determinant(viewProj), viewProj));
		m_PassUniform.EyePosW = lightPos;
		m_PassUniform.RenderTargetSize = m_ShadowMapSize;
		m_PassUniform.InvRenderTargetSize = { 1.0f / m_ShadowMapSize.x, 1.0f / m_ShadowMapSize.y };
		m_PassUniform.NearZ = mLightNearZ;
		m_PassUniform.FarZ = mLightFarZ;

		rhi->UpdatePassUniform(1, &m_PassUniform);

		m_PassUniform.ShadowTransform = shadowTransform; //留给forwardpass 复制使用
	}
	
	auto& renderProxys = renderScene->GetRenderProxys(MaterialBlendMode::Opaque);
	for (auto& proxy : renderProxys)
	{
		auto renderProxy = static_cast<RenderProxy*>(proxy);
		if (renderProxy)
		{
			auto renderItem = rhi->GetRenderItem(renderProxy->GetRenderItemId());
			m_RenderBatch[renderItem->Material ? renderItem->Material->m_Shader->GetShaderId() : 0].push_back(renderProxy->GetRenderItemId());
		}
	}

	for (auto& batch : m_RenderBatch)
	{
		for (auto& renderItemId : batch.second)
		{
			rhi->DrawRenderItem(renderItemId);
		}
	}

	rhi->DrawRenderPass(this, FrameBufferType::ShadowMap);
	m_RenderBatch.clear();
}
