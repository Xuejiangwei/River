#pragma once

#include "RiverHead.h"
#include "Widget.h"

class Text : public Widget
{
public:
	Text();

	virtual ~Text() override;

	virtual void OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices) override;

private:

};
