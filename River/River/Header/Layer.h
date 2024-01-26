#pragma once
#include "KeyCode.h"

class Layer
{
public:
	Layer();

	virtual ~Layer();

	virtual void OnAttach() = 0;

	virtual void OnDetach() = 0;

	virtual void OnUpdate(float deltaTime) = 0;

	virtual bool OnEvent(const class Event& e) = 0;

	virtual void OnRender() {}

	virtual bool IsUILayer() const { return true; }

	//Event
	virtual bool OnMousePresse() { return false; }

	virtual bool OnMouseRelease() { return false; }
	
	virtual bool OnMouseDrag() { return false; }

	virtual bool OnKeyDown() { return false; }

	virtual bool OnKeyRelease() { return false; }
	
private:
};
