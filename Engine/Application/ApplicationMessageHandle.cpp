#include "ApplicationMessageHandle.h"
#include <iostream>
#include "application.h"

ApplicationMessageHandle::ApplicationMessageHandle()
{
	MouseButtonDownDelegate.AddLambda([](EMouseButton MouseEvent, Vector2d MousePosition, int a, int b)
	{
		std::cout << a << " " << b << std::endl;
	}, 50, 60);
}

void ApplicationMessageHandle::Tick(float DeltaTime)
{
	
}

bool ApplicationMessageHandle::HandleMouseButtonDown(EMouseButton MouseEvent, Vector2d MousePosition)
{
	SetCapture(Application::Get()->m_hWnd);

	if (MouseButtonDownDelegate.IsNotEmpty())
	{
		MouseButtonDownDelegate.Broadcast(MouseEvent, MousePosition);
	}

	std::cout << "鼠标按下" << std::endl;
	std::cout << "x: " << MousePosition.x << ", y: " << MousePosition.y << std::endl;
	return false;
}

bool ApplicationMessageHandle::HandleMouseButtonUp(EMouseButton MouseEvent, Vector2d MousePosition)
{
	ReleaseCapture();

	if (MouseButtonUpDelegate.IsNotEmpty())
	{
		MouseButtonUpDelegate.Broadcast(MouseEvent, MousePosition);
	}

	std::cout << "鼠标弹起" << std::endl;
	return false;
}

bool ApplicationMessageHandle::HandleMouseButtonDoubleClick(EMouseButton MouseEvent, Vector2d MousePosition)
{
	if (MouseButtonDoubleClickDelegate.IsNotEmpty())
	{
		MouseButtonDoubleClickDelegate.Broadcast(MouseEvent, MousePosition);
	}

	std::cout << "鼠标双击" << std::endl;
	return false;
}

bool ApplicationMessageHandle::HandleMouseMove(EMouseButton MouseEvent, Vector2d MousePosition)
{
	if (MouseMoveDelegate.IsNotEmpty())
	{
		MouseMoveDelegate.Broadcast(MouseEvent, MousePosition);
	}

	return false;
}

bool ApplicationMessageHandle::HandleMouseWheel(float WheelValue, Vector2d MousePosition)
{
	if (MouseWheelDelegate.IsNotEmpty())
	{
		MouseWheelDelegate.Broadcast(WheelValue, MousePosition);
	}

	std::cout << WheelValue << std::endl;
	return false;
}

bool ApplicationMessageHandle::HandleKeyDown(int ASCII, Vector2d MousePosition)
{
	if (KeyDownDelegate.IsNotEmpty())
	{
		KeyDownDelegate.Broadcast(ASCII, MousePosition);
	}

	std::cout << ASCII << std::endl;
	return false;
}

bool ApplicationMessageHandle::HandleKeyUp(int ASCII, Vector2d MousePosition)
{
	if (KeyUpDelegate.IsNotEmpty())
	{
		KeyUpDelegate.Broadcast(ASCII, MousePosition);
	}

	return false;
}
