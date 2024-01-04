#pragma once

#include "RiverHead.h"

enum class FrameBufferType : uint8
{
	Color,
	ShadowMap,
	UI,
};

class FrameBuffer
{
public:
	FrameBuffer();
	
	virtual ~FrameBuffer();

private:
};