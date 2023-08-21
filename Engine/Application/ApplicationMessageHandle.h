#pragma once
#include "IInputProcessor.h"
#include "../../Core/Delegate.h"

DECLARE_MULTICAST_DELEGATE(FMouseDelegate, EMouseButton, Vector2d);
DECLARE_MULTICAST_DELEGATE(FKeyDelegate, int, Vector2d);
DECLARE_MULTICAST_DELEGATE(FMouseWheelDelegate, float, Vector2d);

class ApplicationMessageHandle : public IInputProcessor
{
public:
	ApplicationMessageHandle();
	~ApplicationMessageHandle(){}

	virtual void Tick(float DeltaTime) override;

	virtual bool HandleMouseButtonDown(EMouseButton MouseEvent, Vector2d MousePosition) override;
	virtual bool HandleMouseButtonUp(EMouseButton MouseEvent, Vector2d MousePosition) override;
	virtual bool HandleMouseButtonDoubleClick(EMouseButton MouseEvent, Vector2d MousePosition) override;

	virtual bool HandleMouseMove(EMouseButton MouseEvent, Vector2d MousePosition) override;
	virtual bool HandleMouseWheel(float WheelValue, Vector2d MousePosition) override;

	virtual bool HandleKeyDown(int ASCII, Vector2d MousePosition) override;
	virtual bool HandleKeyUp(int ASCII, Vector2d MousePosition) override;


	FMouseDelegate MouseButtonDownDelegate;
	FMouseDelegate MouseButtonUpDelegate;
	FMouseDelegate MouseButtonDoubleClickDelegate;
	FMouseDelegate MouseMoveDelegate;

	FMouseWheelDelegate MouseWheelDelegate;

	FKeyDelegate KeyDownDelegate;
	FKeyDelegate KeyUpDelegate;
};

