#include "application.h"
#include "IInputProcessor.h"
#include <windowsx.h>
#include "ApplicationMessageHandle.h"
#include <iostream>
#pragma comment(lib, "d2d1")

Application* Application::app = nullptr;

HRESULT Application::CreateGraphicsResource(HWND hWnd)
{
	HRESULT hr = S_OK;
	if (pRenderTarget == nullptr)
	{
		RECT rc;
		GetClientRect(hWnd, &rc);
		D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

		if (pFactory)
		{
			hr = pFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hWnd, size), &pRenderTarget);
		}
		if (SUCCEEDED(hr))
		{
			hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray), &GrayBrush);
		}
		if (SUCCEEDED(hr))
		{
			D2D1::ColorF cof(1, 0, 0);
			hr = pRenderTarget->CreateSolidColorBrush(cof, &BlueBrush);
		}
	}
	return hr;
}

void Application::ReleaseAllResource()
{
	SafeRelease(&pRenderTarget);
	SafeRelease(&GrayBrush);
	SafeRelease(&BlueBrush);
}

void Application::OnPaint(HWND hWnd)
{
	HRESULT hr = Get()->CreateGraphicsResource(hWnd);
	if (SUCCEEDED(hr))
	{
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		if (Get()->pRenderTarget)
		{
			D2D1_SIZE_F size = pRenderTarget->GetSize();
			Get()->pRenderTarget->BeginDraw();
			Get()->pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
			Get()->pRenderTarget->DrawLine(D2D1::Point2F(100, 100), D2D1::Point2F(500, 500), Get()->GrayBrush, 1.0f);

			Get()->pRenderTarget->DrawLine(D2D1::Point2F(0, size.height / 2), D2D1::Point2F(size.width, size.height / 2), Get()->BlueBrush, 1.0f);
			Get()->pRenderTarget->DrawLine(D2D1::Point2F(size.width / 2, 0), D2D1::Point2F(size.width / 2, size.height), Get()->BlueBrush, 1.0f);

			hr = Get()->pRenderTarget->EndDraw();
			if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
			{
				Get()->ReleaseAllResource();
			}
		}
		EndPaint(hWnd, &ps);

	}
}

Application* Application::Get()
{
	if (!app)
	{
		app = new Application();
	}
	return app;
}

Application::Application()
{
	MainInputProcessor = new ApplicationMessageHandle();
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
	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);

	return true;
}

bool Application::Run()
{
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg); //翻译消息
		DispatchMessage(&msg);  //转发给消息处理函数

		Tick(0);
	}

	return (int)msg.wParam;
}

void Application::Tick(float DeltaTime)
{
	MainInputProcessor->Tick(DeltaTime);
	for (IInputProcessor* InputProcessor : InputProcessors)
	{
		InputProcessor->Tick(DeltaTime);
	}
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

EKeyState Application::GetControlKeyState()
{
	return bPressedControl ? EKeyState::IE_Pressed : EKeyState::IE_Released;
}

EKeyState Application::GetShiftKeyState()
{
	return bPressedShift ? EKeyState::IE_Pressed : EKeyState::IE_Released;
}

LRESULT CALLBACK Application::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//ClientToScreen,客户端坐标到屏幕坐标
	//ClipCursor,限制鼠标在矩形内移动，矩形坐标为屏幕坐标
	//DragDetect,是否在拖动

	Get()->bPressedControl = (wParam & MK_CONTROL) ? false : true;
	Get()->bPressedShift = (wParam & MK_SHIFT) ? false : true;
	switch (msg)
	{
	//创建时
	case WM_CREATE:
	{
		if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &(Get()->pFactory))))
		{
			return -1;
		}
		break;
	}
	case WM_DESTROY:
	{
		Get()->SafeRelease(&Get()->pFactory);
		PostQuitMessage(0);
	}

	case WM_PAINT:
	{
		Get()->OnPaint(hWnd);
		break;
	}

	case WM_SIZE:
	{
		if (Get()->pRenderTarget)
		{
			RECT rc;
			GetClientRect(hWnd, &rc);
			D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
			Get()->pRenderTarget->Resize(size);
		}

		break;
	}

	//鼠标事件
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_LBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONDBLCLK:
	case WM_MBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONDBLCLK:
	case WM_RBUTTONUP:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONDBLCLK:
	case WM_XBUTTONUP:
	{
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		const Vector2d MousePosition = Vector2d(xPos, yPos);
		EMouseButton MouseButton = EMouseButton::Invalid;
		bool bMouseUp = false;
		bool bDoubleClick = false;
		switch (msg)
		{
		case WM_LBUTTONDOWN:
			MouseButton = EMouseButton::Left;
			break;
		case WM_LBUTTONDBLCLK:
			MouseButton = EMouseButton::Left;
			bDoubleClick = true;
			break;
		case WM_LBUTTONUP:
			MouseButton = EMouseButton::Left;
			bMouseUp = true;
			break;
		case WM_MBUTTONDOWN:
			MouseButton = EMouseButton::Middle;
			break;
		case WM_MBUTTONDBLCLK:
			MouseButton = EMouseButton::Middle;
			bDoubleClick = true;
			break;
		case WM_MBUTTONUP:
			MouseButton = EMouseButton::Middle;
			bMouseUp = true;
			break;
		case WM_RBUTTONDOWN:
			MouseButton = EMouseButton::Right;
			break;
		case WM_RBUTTONDBLCLK:
			MouseButton = EMouseButton::Right;
			bDoubleClick = true;
			break;
		case WM_RBUTTONUP:
			MouseButton = EMouseButton::Right;
			bMouseUp = true;
			break;
		case WM_XBUTTONDOWN:
			MouseButton = GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? EMouseButton::XButton01 : EMouseButton::XButton02;
			break;
		case WM_XBUTTONDBLCLK:
			MouseButton = GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? EMouseButton::XButton01 : EMouseButton::XButton02;
			bDoubleClick = true;
			break;
		case WM_XBUTTONUP:
			MouseButton = GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? EMouseButton::XButton01 : EMouseButton::XButton02;
			bMouseUp = true;
			break;
		}
		if (bMouseUp)
		{
			Get()->MainInputProcessor->HandleMouseButtonUp(MouseButton, MousePosition);
			for (IInputProcessor* InputProcessor : Get()->InputProcessors)
			{
				InputProcessor->HandleMouseButtonUp(MouseButton, MousePosition);
			}
		}
		else if (bDoubleClick)
		{
			Get()->MainInputProcessor->HandleMouseButtonDoubleClick(MouseButton, MousePosition);
			for (IInputProcessor* InputProcessor : Get()->InputProcessors)
			{
				InputProcessor->HandleMouseButtonDoubleClick(MouseButton, MousePosition);
			}
		}
		else
		{
			Get()->MainInputProcessor->HandleMouseButtonDown(MouseButton, MousePosition);
			for (IInputProcessor* InputProcessor : Get()->InputProcessors)
			{
				InputProcessor->HandleMouseButtonDown(MouseButton, MousePosition);
			}
		}
		break;
	}

	//鼠标移动
	case WM_NCMOUSEMOVE:
	case WM_MOUSEMOVE:
	{
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		const Vector2d MousePosition = Vector2d(xPos, yPos);

		Get()->MainInputProcessor->HandleMouseMove(EMouseButton::Invalid, MousePosition);
		for (IInputProcessor* InputProcessor : Get()->InputProcessors)
		{
			InputProcessor->HandleMouseButtonDown(EMouseButton::Invalid, MousePosition);
		}

		break;
	}

	//鼠标滚轮
	case WM_MOUSEWHEEL:
	{
		const SHORT WheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		const Vector2d MousePosition = Vector2d(xPos, yPos);

		Get()->MainInputProcessor->HandleMouseWheel(static_cast<float>(WheelDelta) / 120.f, MousePosition);
		for (IInputProcessor* InputProcessor : Get()->InputProcessors)
		{
			InputProcessor->HandleMouseWheel(static_cast<float>(WheelDelta) / 120.f, MousePosition);
		}

		break;
	}

	//键盘输入
	case WM_KEYDOWN:
	case WM_KEYUP:
	{
		int ASCII = static_cast<int>(wParam);
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		const Vector2d MousePosition = Vector2d(xPos, yPos);
		if (msg == WM_KEYDOWN)
		{
			Get()->MainInputProcessor->HandleKeyDown(ASCII, MousePosition);
			for (IInputProcessor* InputProcessor : Get()->InputProcessors)
			{
				InputProcessor->HandleKeyDown(ASCII, MousePosition);
			}
		}
		else
		{
			Get()->MainInputProcessor->HandleKeyUp(ASCII, MousePosition);
			for (IInputProcessor* InputProcessor : Get()->InputProcessors)
			{
				InputProcessor->HandleKeyUp(ASCII, MousePosition);
			}
		}

		break;
	}

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

bool Application::RegisterWindow(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(WNDCLASSEX));

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
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
	this->m_hWnd = hWnd;
	return true;
}
