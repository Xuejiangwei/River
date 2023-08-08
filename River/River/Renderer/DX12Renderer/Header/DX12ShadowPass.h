#pragma once

#include "RenderPass.h"

class DX12ShadowPass : public RenderPass
{
public:
	DX12ShadowPass();

	virtual ~DX12ShadowPass() override;

	virtual void Initialize() override;

	virtual void Draw() override;

	virtual void Update() override;

private:

};
