#include "application.h"
#include "IInputProcessor.h"
#include <windowsx.h>
#include "ApplicationMessageHandle.h"
#include <iostream>
#include "../../Core/d3d12Util.h"
#include "../../Core/d3dx12.h"
#include <assert.h>
#include "../../Core/GameTime.h"

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

		if (pD2DFactory)
		{
			hr = pD2DFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hWnd, size), &pRenderTarget);
		}
	}
	return hr;
}

void Application::ReleaseAllResource()
{
	SafeRelease(&pRenderTarget);
}

void Application::OnPaint(HWND hWnd)
{
	HRESULT hr = S_OK; 
	if (pRenderTarget == nullptr)
	{
		hr = CreateGraphicsResource(hWnd);
	}
	if (pRenderTarget && SUCCEEDED(hr))
	{
		D2D1_SIZE_F size = pRenderTarget->GetSize();

		pRenderTarget->BeginDraw();
		pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

		DrawLine(Vector2f(100, 100), Vector2f(500, 500), Color(1, 0, 0), 1.0f);
		DrawLine(Vector2f(0, size.height / 2), Vector2f(size.width,size.height / 2), Color(0, 0, 0), 1.0f);
		DrawLine(Vector2f(size.width / 2, 0), Vector2f(size.width / 2,size.height), Color(0.5, 0.5, 0.5), 1.0f);

		pRenderTarget->EndDraw();
	}
}

HRESULT Application::DrawLine(Vector2f Start, Vector2f End, Color c, float width)
{
	HRESULT hr = S_OK;
	if (pD2DFactory && pRenderTarget)
	{
		D2D1::ColorF col(c.x, c.y, c.z, c.w);
		ID2D1SolidColorBrush* Brush;
		hr = pRenderTarget->CreateSolidColorBrush(col, &Brush);
		if (SUCCEEDED(hr))
		{
			pRenderTarget->DrawLine(D2D1::Point2F(Start.x, Start.y), D2D1::Point2F(End.x, End.y), Brush, width);
		}
	}
	return hr;
}

Application* Application::Get()
{
	if (!app)
	{
		app = new Application(800, 600);
	}
	return app;
}

Application::Application(int width, int height)
{
	m_width = width;
	m_height = height;

	MainInputProcessor = new ApplicationMessageHandle();
}

bool Application::Init(HINSTANCE hInstance)
{
	if (!InitWindow(hInstance))
	{
		return false;
	}

	if (!InitDirectX())
	{
		return false;
	}
	gt = new GameTime();
	return true;
}

bool Application::InitWindow(HINSTANCE hInstance)
{
	if (!RegisterWindow(hInstance))
	{
		return false;
	}
	if (!CreateMyWindow(hInstance))
	{
		return false;
	}
	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);


	return true;
}

bool Application::InitDirectX()
{
	//开启D3D12调试信息
#if defined(DEBUG) || defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debugController;
		ThrowIfFailedM(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)), L"Open Debug Message Failed");
		debugController->EnableDebugLayer();
	}
#endif

	CreateDevice();//创建设备
	CreateFence();//创建围栏,同步CPU和GPU
	GetDescriptorSize();//获得描述符大小
	SetMSAA();
	CreateCommandObject();//创建命令队列，命令分配器，命令列表
	CreateSwapChain();//创建交换链
	CreateDescriptorHeap();//创建描述符堆

	OnResize();

	return true;
}

void Application::OnResize()
{
	if (!pCommandList)
	{
		return;
	}

	FlushCommandQueue();

	ThrowIfFailedM(pCommandList->Reset(pCommandAllocator.Get(), nullptr), L"Resize Command List Failed");

	for (int i = 0; i < 2; i++)
	{
		swapChainBuffer[i].Reset();
	}
	depthStencilBuffer.Reset();

	ThrowIfFailedM(pSwapChain->ResizeBuffers(2, m_width, m_height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH), L"Resize Swap Chain Failed");

	mCurrentBackBuffer = 0;

	CreateRTV();
	CreateDSV();
	CreateViewportAndScissorRect();
}

bool Application::Run()
{
	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg); //翻译消息
			DispatchMessage(&msg);  //转发给消息处理函数
		}
		else
		{
			if (gt)
			{
				gt->Tick();
				CalculateFrameState();
			}
			Tick(0);
			Draw();
		}
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

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return Application::Get()->WndProc(hWnd, msg, wParam, lParam);
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
		if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &(Get()->pD2DFactory))))
		{
			return -1;
		}
		break;
	}
	case WM_DESTROY:
	{
		Get()->SafeRelease(&Get()->pD2DFactory);
		PostQuitMessage(0);
	}

	case WM_PAINT:
	{
		Get()->OnPaint(hWnd);
		break;
	}

	case WM_SIZE:
	{
		m_width = LOWORD(lParam);
		m_height = HIWORD(lParam);
		if (pDevice)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				bMinimized = true;
				bMaximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				bMinimized = false;
				bMaximized = true;
				OnResize();
			}
			else if (wParam == SIZE_RESTORED)
			{
				if (bMinimized)
				{
					bMinimized = false;
					OnResize();
				}
				else if (bMaximized)
				{
					bMaximized = false;
					OnResize();
				}
				else if (bResizing)
				{

				}
				else
				{
					OnResize();
				}
			}
		}
		if (Get()->pRenderTarget)
		{
			RECT rc;
			GetClientRect(hWnd, &rc);
			D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
			Get()->pRenderTarget->Resize(size);
		}
		break;
	}
	case WM_ENTERSIZEMOVE:
	{
		bResizing = true;
		break;
	}
	case WM_EXITSIZEMOVE:
	{
		bResizing = false;
		OnResize();
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
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

bool Application::RegisterWindow(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(WNDCLASSEX));

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc = MainWndProc;
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

bool Application::CreateMyWindow(HINSTANCE hInstance)
{
	RECT R = { 0, 0, m_width, m_height };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	HWND hWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, L"MyWindowClass1", L"Engine", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, hInstance, NULL);
	if (!hWnd)
	{
		MessageBox(NULL, L"创建窗口失败!", L"Error", MB_OK | MB_ICONSTOP);
		return false;
	}
	m_hWnd = hWnd;
	return true;
}

template<typename T>
void Application::SafeRelease(T** ppInterfaceToRelease)
{
	if (*ppInterfaceToRelease != nullptr)
	{
		(*ppInterfaceToRelease)->Release();
		(*ppInterfaceToRelease) = nullptr;
	}
}

void Application::CreateDevice()
{
	//创建dxgi工厂
	ThrowIfFailedM(CreateDXGIFactory1(IID_PPV_ARGS(&pDXGIFactory)), L"Create DXGIFactory Failed!");

	//根据工厂创建设备
	ThrowIfFailed(D3D12CreateDevice(nullptr, //nullptr为主适配器
		D3D_FEATURE_LEVEL_12_0,				 //级别
		IID_PPV_ARGS(&pDevice))
	);
}

void Application::CreateFence()
{
	if (pDevice)
	{
		ThrowIfFailed(pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence)));
	}
}

void Application::GetDescriptorSize()
{
	//描述符大小可用于后续对描述符堆进行偏移，进而获得后面的描述符
	if (pDevice)
	{
		rtvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		dsvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		cbv_srv_uavDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
}

void Application::SetMSAA()
{
	if (pDevice)
	{
		msaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		msaaQualityLevels.SampleCount = 4;
		msaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
		msaaQualityLevels.NumQualityLevels = 0;

		ThrowIfFailed(pDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaQualityLevels, sizeof(msaaQualityLevels)));
		//NumQualityLevels在Check函数里会进行设置
		//如果支持MSAA，则Check函数返回的NumQualityLevels > 0
		assert(msaaQualityLevels.NumQualityLevels > 0);
	}
}

void Application::CreateCommandObject()
{
	if (pDevice)
	{
		//创建命令队列
		D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
		commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		ThrowIfFailedM(pDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&pCommandQueue)), L"Create Command Queue Failed");

		//创建命令分配器
		ThrowIfFailedM(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pCommandAllocator)), L"Create Command Allocator Failed");

		//创建命令列表
		ThrowIfFailedM(pDevice->CreateCommandList(
			0, //掩码值为0，单GPU
			D3D12_COMMAND_LIST_TYPE_DIRECT, //命令列表类型
			pCommandAllocator.Get(),	//命令分配器指针
			nullptr,						//渲染流水线状态对象PSO(pipeline state object),这里不绘制为nullptr
			IID_PPV_ARGS(&pCommandList)
		), L"Create Command List Failed"
		);

		//首先需要重置命令列表，因为在第一次引用命令列表时需要对它进行重置，而重置前必须先关闭
		pCommandList->Close();
	}
}

void Application::CreateSwapChain()
{
	if (pDXGIFactory)
	{
		pSwapChain.Reset();
		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		swapChainDesc.BufferDesc.Width = m_width;		//缓冲区大小
		swapChainDesc.BufferDesc.Height = m_height;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	//缓冲区格式
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;			//缓冲区刷新率
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;	//扫描方式,逐行扫描VS隔行扫描(未指定的)
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;//图像相对屏幕的拉伸（未指定的）
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//将数据渲染到后台缓冲区，即作为渲染目标
		swapChainDesc.OutputWindow = m_hWnd;			//渲染窗口句柄
		swapChainDesc.SampleDesc.Count = 1;				//多重采样数量
		swapChainDesc.SampleDesc.Quality = 0;			//多重采样质量
		swapChainDesc.Windowed = true;					//是否窗口化
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	//固定写法
		swapChainDesc.BufferCount = 2;					//缓冲区数量
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;	//自适应窗口模式,自动选择最适于当前窗口尺寸的显示模式

		ThrowIfFailed(pDXGIFactory->CreateSwapChain(pCommandQueue.Get(), &swapChainDesc, pSwapChain.GetAddressOf()));
	}
}

void Application::CreateDescriptorHeap()
{
	//先创建render target view堆
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc;
	rtvDescriptorHeapDesc.NumDescriptors = 2;
	rtvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDescriptorHeapDesc.NodeMask = 0;
	ThrowIfFailedM(pDevice->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap)), L"Create rtv Descriptor Heap Failed");

	//再创建depth stencil view堆
	D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorDesc;
	dsvDescriptorDesc.NumDescriptors = 1;
	dsvDescriptorDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvDescriptorDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvDescriptorDesc.NodeMask = 0;
	ThrowIfFailedM(pDevice->CreateDescriptorHeap(&dsvDescriptorDesc, IID_PPV_ARGS(&dsvDescriptorHeap)), L"Create dsv Descriptor Heap Failed");
}

void Application::CreateRTV()
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	for (int i = 0; i < 2; i++)
	{
		//获得存在于交换链中的第i个缓冲区
		pSwapChain->GetBuffer(i, IID_PPV_ARGS(&swapChainBuffer[i]));

		//为此缓冲区创建一个RTV
		pDevice->CreateRenderTargetView(
			swapChainBuffer[i].Get(),
			nullptr,	//创建交换链时已经定义了该资源的数据格式,所以这里给空指针
			rtvHeapHandle
		);

		//偏移到描述符堆的下一个缓冲区
		rtvHeapHandle.Offset(1, rtvDescriptorSize);
	}
}

void Application::CreateDSV()
{
	D3D12_RESOURCE_DESC dsvResourceDesc;
	dsvResourceDesc.Alignment = 0;	//指定对齐
	dsvResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;	//指定资源维度
	dsvResourceDesc.DepthOrArraySize = 1;	//纹理深度为1；
	dsvResourceDesc.Width = m_width;
	dsvResourceDesc.Height = m_height;
	dsvResourceDesc.MipLevels = 1;	//mipmap层级
	dsvResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;	//指定纹理布局，这里不指定
	dsvResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;	//指定为深度模板资源的Flag
	dsvResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	//24位深度，8位模板
	dsvResourceDesc.SampleDesc.Count = 1;	//多重采样数量
	dsvResourceDesc.SampleDesc.Quality = msaaQualityLevels.NumQualityLevels - 1;

	CD3DX12_CLEAR_VALUE optClear;	//清除资源的优化值，提高清除操作的执行速度（CreateCommittedResource函数中传入）
	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	CD3DX12_HEAP_PROPERTIES HeapProperties(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(pDevice->CreateCommittedResource(
		&HeapProperties,//堆类型为默认堆（不能写入）
		D3D12_HEAP_FLAG_NONE,
		&dsvResourceDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(&depthStencilBuffer)
	)
	);

	//创建DSV(必须填充DSV属性结构体，和创建RTV不同，RTV是通过句柄)
	//D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	//dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	//dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	//dsvDesc.Texture2D.MipSlice = 0;
	pDevice->CreateDepthStencilView(
		depthStencilBuffer.Get(),
		nullptr,//D3D12_DEPTH_STENCIL_VIEW_DESC类型指针，可填&dsvDesc（见上注释代码）由于在创建深度模板资源时已经定义深度模板数据属性，所以这里可以指定为空指针
		dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
	);

	//将资源从初始状态转换为深度缓冲区
	CD3DX12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(depthStencilBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	pCommandList->ResourceBarrier(1, &Barrier);

	ThrowIfFailedM(pCommandList->Close(), L"InitDirectX12 Command List Close Failed");
	ID3D12CommandList* cmdLists[] = { pCommandList.Get() }; //声明并定义命令列表数组
	pCommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);//将命令从命令列表传至命令队列

	FlushCommandQueue();
}

void Application::FlushCommandQueue()
{
	if (pCommandQueue)
	{
		mCurrentFence++;
		pCommandQueue->Signal(pFence.Get(), mCurrentFence);
		if (pFence->GetCompletedValue() < mCurrentFence)
		{
			HANDLE eventHandle = CreateEvent(nullptr, false, false, L"FenceSetDone");
			pFence->SetEventOnCompletion(mCurrentFence, eventHandle);
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
	}
}

void Application::CreateViewportAndScissorRect()
{
	//视口设置
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Height = static_cast<float>(m_height);
	viewport.Width = static_cast<float>(m_width);
	viewport.MaxDepth = 1;
	viewport.MinDepth = 0;

	//裁剪矩形设置
	scissorRect = { 0, 0, m_width, m_height };
}

void Application::Draw()
{
	//重新分配命令分配器和命令列表
	ThrowIfFailed(pCommandAllocator->Reset());
	ThrowIfFailed(pCommandList->Reset(pCommandAllocator.Get(), nullptr));

	UINT& ref_mCurrentBackBuffer = mCurrentBackBuffer;

	//对当前的缓冲区操作，由呈现状态转换为渲染状态
	CD3DX12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(swapChainBuffer[ref_mCurrentBackBuffer].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	pCommandList->ResourceBarrier(1, &Barrier);

	//重新设置视口和裁剪矩形
	pCommandList->RSSetViewports(1, &viewport);
	pCommandList->RSSetScissorRects(1, &scissorRect);

	//先找到当前的rtv和dsv，然后清理当前render target view和depth stencil view
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), ref_mCurrentBackBuffer, rtvDescriptorSize);
	const float color[4] = { 0.69f, 0.77f, 0.87f, 1.f }; //浅蓝色
	//const float color[4] = { 1, 1, 1, 1.f }; //浅蓝色
	pCommandList->ClearRenderTargetView(
		rtvHandle,
		color,	//清理后的颜色
		0,		//裁剪矩形数量
		nullptr	//裁剪矩形指针
	);
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	pCommandList->ClearDepthStencilView(
		dsvHandle,
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.0f,	//清理后的深度值
		0,		//清理后的模板值
		0,		//裁剪矩形数量
		nullptr	//裁剪矩形指针
	);

	//重新指定RenderTargetView和DepthStencilView
	pCommandList->OMSetRenderTargets(1, &rtvHandle, true, &dsvHandle);

	//由渲染状态转换为呈现状态
	CD3DX12_RESOURCE_BARRIER Barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(swapChainBuffer[ref_mCurrentBackBuffer].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	pCommandList->ResourceBarrier(1, &Barrier2);
	//完成命令的记录关闭命令列表
	ThrowIfFailed(pCommandList->Close());

	ID3D12CommandList* commandLists[] = { pCommandList.Get() };//声明并定义命令列表数组
	pCommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);//将命令从命令列表传至命令队列

	//交互缓冲区
	ThrowIfFailed(pSwapChain->Present(0, 0));
	ref_mCurrentBackBuffer = (ref_mCurrentBackBuffer + 1) % 2;

	FlushCommandQueue();
}

void Application::CalculateFrameState()
{
	mFrameCount++;

	if (gt && gt->GetTotalTime() - timeElapsed >= 1.0f)
	{
		float fps = float(mFrameCount); //每秒多少帧
		float mspf = 1000.f / fps; //每帧多少毫秒

		std::wstring fpsStr = std::to_wstring(fps);
		std::wstring mspfStr = std::to_wstring(mspf);
		std::wstring windowTitle = L"D3D12    fsp:" + fpsStr + L"    mspf:" + mspfStr;
		SetWindowText(m_hWnd, windowTitle.c_str());
		mFrameCount = 0;
		timeElapsed += 1.0f;
	}
}
