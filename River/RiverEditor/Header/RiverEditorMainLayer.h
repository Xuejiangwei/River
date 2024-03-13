#pragma once
#include "RiverHead.h"
#include "Layer.h"

class Object;
class CameraObject;

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
	bool OnMousePress(const Event& e);

	bool OnMouseRelease(const Event& e);

	bool OnMouseMove(const Event& e);

	bool OnMouseLeave();

	bool OnKeyPress(const Event& e);

	bool OnKeyRelease(const Event& e);

private:
	V_Array<Share<Object>> m_EditorObjects;
	Share<CameraObject> m_MainCamera;
};
