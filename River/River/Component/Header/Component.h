#pragma once

#include "RiverHead.h"

class Component
{
public:
	Component() {}

	virtual ~Component() {}

	virtual void Tick(float deltaTime);

private:

};