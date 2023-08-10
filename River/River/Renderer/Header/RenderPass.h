#pragma once

class RenderPass
{
public:
	virtual ~RenderPass() {}

	virtual void Initialize() = 0;

	virtual void Draw() = 0;

	virtual void Update(class FrameBuffer* frameBuffer) = 0;

	virtual void OnResize(int width, int height) = 0;
};