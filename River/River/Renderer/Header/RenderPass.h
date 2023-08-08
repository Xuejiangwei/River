#pragma once

class RenderPass
{
public:
	virtual ~RenderPass();

	virtual void Initialize() = 0;

	virtual void Draw() = 0;

	virtual void Update() = 0;
};