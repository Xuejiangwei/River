#pragma once

#include "RiverHead.h"
#include "Widget.h"

class Image : public Widget
{
public:
	Image();

	virtual ~Image() override;

	virtual void OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices) override;

	void SetTexture(const char* texturePath);

private:
	const char* m_Texture;
};
