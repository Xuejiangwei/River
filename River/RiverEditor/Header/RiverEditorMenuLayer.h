#pragma once

#include "Layer.h"
#include "Renderer/Mesh/Header/Mesh.h"

class UIWindow;

class RiverEditorMenuLayer : public Layer
{
public:
	RiverEditorMenuLayer();

	virtual ~RiverEditorMenuLayer() override;

	virtual void OnInitialize() override;

	virtual void OnAttach() override;

	virtual void OnDetach() override;

	virtual void OnUpdate(float deltaTime) override;

	virtual bool OnEvent(const class Event& e) override;

	virtual void OnRender() override;

private:
	LessRBTreeMap<int, Unique<UIWindow>> m_UIWindows;
};
