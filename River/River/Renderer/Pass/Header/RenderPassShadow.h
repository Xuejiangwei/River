#pragma once
#include "RenderPass.h"

class Texture;

class RenderPassShadow : public RenderPass
{
public:
	RenderPassShadow();

	virtual ~RenderPassShadow() override;

	virtual void Render() override;

	Texture* GetShadowMapTexture() { return m_ShadowMapTexture; }

private:
	int m_CommandId;
	Float2 m_ShadowMapSize;
	Texture* m_ShadowMapTexture;
	RenderPass::PassUniform m_PassUniform;
	HashMap<int, V_Array<int>> m_RenderBatch;
};
