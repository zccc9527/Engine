#pragma once
#include <windows.h>
#include "../Math/vector.h"

enum class EMouseButton
{
	Invalid = 0,
	Left,
	Middle,
	Right,
	XButton01,
	XButton02
};

enum class EKeyState
{
	Invalid = 0,
	IE_Pressed,
	IE_Released
};

class IInputProcessor
{
public:
	IInputProcessor(){}
	virtual ~IInputProcessor(){}

	virtual void Tick(float DeltaTime) = 0;

	virtual bool HandleMouseButtonDown(EMouseButton MouseEvent, Vector2d MousePosition) { return false; }
	virtual bool HandleMouseButtonUp(EMouseButton MouseEvent, Vector2d MousePosition) { return false; }
	virtual bool HandleMouseButtonDoubleClick(EMouseButton MouseEvent, Vector2d MousePosition) { return false; }

	virtual bool HandleMouseMove(EMouseButton MouseEvent, Vector2d MousePosition) { return false; }
	virtual bool HandleMouseWheel(float WheelValue, Vector2d MousePosition) { return false; }

	virtual bool HandleKeyDown(int ASCII, Vector2d MousePosition) { return false; }
	virtual bool HandleKeyUp(int ASCII, Vector2d MousePosition) { return false; }
};

