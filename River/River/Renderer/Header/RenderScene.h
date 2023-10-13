#pragma once
#include "RiverHead.h"

class RenderProxy;

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
};
