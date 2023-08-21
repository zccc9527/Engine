#include "ApplicationMessageHandle.h"
#include <iostream>
#include "application.h"

void ApplicationMessageHandle::Tick(float DeltaTime)
{
	
}

bool ApplicationMessageHandle::HandleMouseButtonDown(EMouseButton MouseEvent, Vector2d MousePosition)
{
	SetCapture(Application::Get()->hWnd);
	std::cout << "��갴��" << std::endl;
	std::cout << "x: " << MousePosition.x << ", y: " << MousePosition.y << std::endl;
	return false;
}

bool ApplicationMessageHandle::HandleMouseButtonUp(EMouseButton MouseEvent, Vector2d MousePosition)
{
	ReleaseCapture();
	std::cout << "��굯��" << std::endl;
	return false;
}

bool ApplicationMessageHandle::HandleMouseButtonDoubleClick(EMouseButton MouseEvent, Vector2d MousePosition)
{
	std::cout << "���˫��" << std::endl;
	return false;
}

bool ApplicationMessageHandle::HandleMouseMove(EMouseButton MouseEvent, Vector2d MousePosition)
{
	return false;
}

bool ApplicationMessageHandle::HandleMouseWheel(float WheelValue, Vector2d MousePosition)
{
	std::cout << WheelValue << std::endl;
	return false;
}

bool ApplicationMessageHandle::HandleKeyDown(int ASCII, Vector2d MousePosition)
{
	std::cout << ASCII << std::endl;
	return false;
}

bool ApplicationMessageHandle::HandleKeyUp(int ASCII, Vector2d MousePosition)
{
	return false;
}
