#include "application.h"
#include "IInputProcessor.h"

Application* Application::app = nullptr;

Application* Application::Get()
{
	if (!app)
	{
		app = new Application();
	}
	return app;
}

bool Application::Init(HINSTANCE hInstance, const wchar_t* title, int x, int y, int width, int height)
{
	if (!RegisterWindow(hInstance))
	{
		return false;
	}
	if (!CreateMyWindow(hInstance, title, x, y, width, height))
	{
		return false;
	}
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	return true;
}

bool Application::Run()
{
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg); //翻译消息
		DispatchMessage(&msg);  //转发给消息处理函数
	}

	return (int)msg.wParam;
}

void Application::RegisterInputProcessor(IInputProcessor* InputProcessor, const int Index /*= -1*/)
{
	if (Index == -1)
	{
		InputProcessors.emplace_back(InputProcessor);
	}
	else
	{
		InputProcessors.insert(InputProcessors.begin(), InputProcessor);
	}
}

void Application::UnRegisterInputProcessor(IInputProcessor* InputProcessor)
{
	auto it = InputProcessors.begin();
	for (; it != InputProcessors.end(); it++)
	{
		if (*it == InputProcessor)
		{
			break;
		}
	}
	if (*it == InputProcessor)
	{
		InputProcessors.erase(it);
	}
}

LRESULT CALLBACK Application::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (app)
	{
		for (IInputProcessor* InputProcessor : app->InputProcessors)
		{
			if (InputProcessor->ProcessMessage(hWnd, msg, wParam, lParam))
			{
				break;
			}
		}
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

bool Application::RegisterWindow(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(hInstance, IDC_ARROW);
	wcex.hbrBackground = HBRUSH(COLOR_WINDOW);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"MyWindowClass1";
	wcex.hIconSm = LoadIcon(hInstance, IDI_APPLICATION);

	this->hInstance = hInstance;

	ATOM WindowId = RegisterClassEx(&wcex);
	if (!WindowId)
	{
		MessageBox(NULL, L"注册窗口失败!", L"Error", MB_OK | MB_ICONSTOP);
		return false;
	}
	return true;
}

bool Application::CreateMyWindow(HINSTANCE hInstance, const wchar_t* title, int x, int y, int width, int height)
{
	HWND hWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, L"MyWindowClass1", title, WS_OVERLAPPEDWINDOW, x, y, width, height, NULL, NULL, hInstance, NULL);
	if (!hWnd)
	{
		MessageBox(NULL, L"创建窗口失败!", L"Error", MB_OK | MB_ICONSTOP);
		return false;
	}
	this->hWnd = hWnd;
	return true;
}
