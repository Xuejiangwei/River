#pragma once

#include "RiverHead.h"
#include "Layer.h"
#include "Renderer/Mesh/Header/Mesh.h"

class UIWindow;

class UILayer : public Layer
{
public:
	UILayer();

	virtual ~UILayer() override;

	virtual void OnAttach() override;

	virtual void OnDetach() override;

	virtual void OnUpdate(float deltaTime) override;

	virtual bool OnEvent(const class Event& e) override;

private:
	GreaterRBTreeMap<int, Unique<UIWindow>> m_UIWindows;

	V_Array<UIVertex> m_RenderVertices;
	V_Array<uint32_t> m_RenderIndices;
};
