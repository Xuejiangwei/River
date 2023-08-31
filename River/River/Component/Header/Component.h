#pragma once

#include "RiverHead.h"
#include "MathStruct.h"

class Component
{
public:
	Component() {}

	virtual ~Component() {}

	virtual void Tick(float deltaTime);

private:

};