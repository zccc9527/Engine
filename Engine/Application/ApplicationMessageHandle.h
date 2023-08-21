#pragma once
#include "IInputProcessor.h"

class ApplicationMessageHandle : public IInputProcessor
{
public:
	ApplicationMessageHandle(){}
	~ApplicationMessageHandle(){}

	virtual void Tick(float DeltaTime) override;

	virtual bool HandleMouseButtonDown(EMouseButton MouseEvent, Vector2d MousePosition) override;
	virtual bool HandleMouseButtonUp(EMouseButton MouseEvent, Vector2d MousePosition) override;
	virtual bool HandleMouseButtonDoubleClick(EMouseButton MouseEvent, Vector2d MousePosition) override;

	virtual bool HandleMouseMove(EMouseButton MouseEvent, Vector2d MousePosition) override;
	virtual bool HandleMouseWheel(float WheelValue, Vector2d MousePosition) override;

	virtual bool HandleKeyDown(int ASCII, Vector2d MousePosition) override;
	virtual bool HandleKeyUp(int ASCII, Vector2d MousePosition) override;
};

