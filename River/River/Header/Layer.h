#pragma once

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

private:
};
