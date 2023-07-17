#pragma once

class Layer
{
public:
	Layer();
	virtual ~Layer();

	virtual void OnUpdate() = 0;

private:

};
