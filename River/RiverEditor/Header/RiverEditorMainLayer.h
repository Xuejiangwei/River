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

	//Event
	virtual bool OnMousePress() override;

	virtual bool OnMouseRelease() override;

	virtual bool OnMouseDrag() override;

	virtual bool OnKeyPress() override;

	virtual bool OnKeyRelease() override;

private:
	V_Array<Share<Object>> m_EditorObjects;
};
