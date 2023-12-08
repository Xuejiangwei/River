#pragma once
#include "RiverHead.h"

class RenderProxy;
class RenderPass;

class RenderScene
{
public:
	RenderScene();

	~RenderScene();

	void OnUpdate();

	void AddObjectProxyToScene(RenderProxy* proxy);

	void RemoveObjectProxyFromScene(RenderProxy* proxy);

private:
	HashSet<void*> m_Proxys;
	V_Array<Share<RenderPass>> m_RenderPasses;
};
