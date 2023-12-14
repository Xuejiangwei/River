#pragma once
#include "RiverHead.h"
#include "Renderer/Header/Material.h"

enum class BlendMode;
class RenderProxy;
class RenderPass;

class RenderScene
{
public:
	RenderScene();

	~RenderScene();

	void Render();

	void AddObjectProxyToScene(RenderProxy* proxy);

	void RemoveObjectProxyFromScene(RenderProxy* proxy);

	Set<void*>& GetRenderProxys(MaterialBlendMode blendMode) { return m_Proxys[blendMode]; }

private:
	HashMap<MaterialBlendMode, Set<void*>> m_Proxys;
	V_Array<Share<RenderPass>> m_RenderPasses;
};
