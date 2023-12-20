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

	void Update(const RiverTime& time);

	void Render();

	int AddObjectProxyToScene(RenderProxy* proxy);

	void RemoveObjectProxyFromScene(RenderProxy* proxy);

	V_Array<RenderProxy*>& GetRenderProxys(MaterialBlendMode blendMode) { return m_Proxys[blendMode]; }

private:
	V_Array<int> m_UnuseProxyId;
	HashMap<MaterialBlendMode, V_Array<RenderProxy*>> m_Proxys;
	V_Array<Share<RenderPass>> m_RenderPasses;
};
