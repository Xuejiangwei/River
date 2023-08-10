#pragma once

class Component
{
public:
	Component() {}

	virtual ~Component() {}

	virtual void Tick(float deltaTime);

private:

};