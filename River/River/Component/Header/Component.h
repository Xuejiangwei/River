#pragma once
#include "RiverHead.h"
#include "Math/Header/BaseStruct.h"

class Component
{
public:
	Component() {}

	virtual ~Component() {}

	virtual void Tick(float deltaTime);

private:

};