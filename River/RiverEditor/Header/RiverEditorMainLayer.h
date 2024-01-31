#pragma once
#include "RiverHead.h"
#include "Layer.h"

class Object;

class RiverEditorMainLayer : public Layer
{
public:
	RiverEditorMainLayer();

	virtual ~RiverEditorMainLayer() override;

	virtual void OnInitialize() override;

	virtual void OnAttach() override;

	virtual void OnDetach() override;

	virtual void OnUpdate(float deltaTime) override;

	virtual bool OnEvent(const class Event& e) override;

	virtual void OnRender() override;

private:
	V_Array<Share<Object>> m_EditorObjects;
};
