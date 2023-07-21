#pragma once

class Layer
{
public:
	Layer();
	virtual ~Layer();

	virtual void OnUpdate() = 0;

	virtual bool OnEvent(const class Event& e) = 0;

private:

};
