#pragma once
#include "RiverHead.h"
#include "Object.h"
#include "Component/Header/SkyBoxComponent.h"

class SkyBoxObject : public Object
{
public:
	SkyBoxObject();

	virtual ~SkyBoxObject() override;

private:
	Share<SkyBoxComponent> m_SkyBoxComponent;
};