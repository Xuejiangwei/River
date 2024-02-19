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
	virtual bool OnMousePress(MouseCode mouseCode, Int2 mousePosition) override;

	virtual bool OnMouseRelease(MouseCode mouseCode, Int2 mousePosition) override;

	virtual bool OnMouseMove(int x, int y) override;

	virtual bool OnMouseLeave() override;

	virtual bool OnKeyPress(KeyCode key) override;

	virtual bool OnKeyRelease(KeyCode key) override;

private:
	V_Array<Share<Object>> m_EditorObjects;
	Share<CameraObject> m_MainCamera;
};
