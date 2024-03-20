#pragma once
#include "UILayer.h"

class RiverMainUiLayer : public UILayer
{
public:
	RiverMainUiLayer();

	virtual ~RiverMainUiLayer() override;

	virtual void OnInitialize() override;

	virtual void OnAttach() override;

	virtual void OnDetach() override;

	virtual void OnUpdate(float deltaTime) override;

	virtual bool OnEvent(const class Event& e) override;

	virtual void OnRender() override;

private:

};
