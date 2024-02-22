#pragma once
#include "RiverHead.h"
#include "Renderer/Header/Material.h"

enum class BlendMode;
class RenderProxy;
class RenderPass;
class RenderPassUI;
class RenderPassShadow;
class Layer;

class RenderScene
{
public:
	RenderScene();

	~RenderScene();

	void Initialize();

	void Update(const RiverTime& time);

	void Render();

	int AddObjectProxyToScene(RenderProxy* proxy);

	int AddLightObjectProxyToScene(RenderProxy* proxy);

	int AddCameraObjectProxyToScene(RenderProxy* proxy);

	int AddSkyBoxObjectProxyToScene(RenderProxy* proxy);

	void RemoveObjectProxyFromScene(RenderProxy* proxy);

	V_Array<RenderProxy*>& GetRenderProxys(MaterialBlendMode blendMode) { return m_Proxys[blendMode]; }

	V_Array<RenderProxy*>& GetRenderLightProxys() { return m_Lights; }

	V_Array<RenderProxy*>& GetRenderCameraProxys() { return m_Cameras; }

	V_Array<RenderProxy*>& GetRenderSkyBoxProxys() { return m_SkyBoxs; }

	void AddUILayer(Share<Layer>& layer);

	RenderPassShadow* GetShadowRenderPass();

	RenderPassUI* GetUIRenderPass();

private:
	V_Array<int> m_UnuseProxyId;
	HashMap<MaterialBlendMode, V_Array<RenderProxy*>> m_Proxys;
	V_Array<RenderProxy*> m_Lights;
	V_Array<RenderProxy*> m_Cameras;
	V_Array<RenderProxy*> m_SkyBoxs;
	V_Array<Share<RenderPass>> m_RenderPasses;
};
