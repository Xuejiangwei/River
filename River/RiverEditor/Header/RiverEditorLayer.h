#pragma once

#include "Layer.h"

class RiverEditorLayer : public Layer
{
public:
	RiverEditorLayer();
	~RiverEditorLayer();

	virtual void OnUpdate() override;

	virtual bool OnEvent(const class Event& e);

private:
};
