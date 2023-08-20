#pragma once
#include <windows.h>

class IInputProcessor
{
public:
	IInputProcessor(){}
	~IInputProcessor(){}

	bool ProcessMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

