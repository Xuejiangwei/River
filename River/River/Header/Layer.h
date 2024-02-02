#pragma once
#include "KeyCode.h"

class Layer
{
public:
	Layer();

	virtual ~Layer();

	virtual void OnInitialize() = 0;

	virtual void OnAttach() = 0;

	virtual void OnDetach() = 0;

	virtual void OnUpdate(float deltaTime) = 0;

	virtual bool OnEvent(const class Event& e) = 0;

	virtual void OnRender() {}

	virtual bool IsUILayer() const { return true; }

	//Event
	virtual bool OnMousePress(MouseCode mouseCode, Int2 mousePosition) { return false; }

	virtual bool OnMouseRelease(MouseCode mouseCode, Int2 mousePosition) { return false; }
	
	virtual bool OnMouseMove(int x, int y) { return false; }

	virtual bool OnKeyPress(KeyCode key) { return false; }

	virtual bool OnKeyRelease(KeyCode key) { return false; }
	
private:
};
