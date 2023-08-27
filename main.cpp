#include <iostream>
#include "Engine/Math/vector.h"
#include "Engine/Math/matrix.h"
#include "Engine/Application/application.h"
#include <windows.h>
#include "Core/Delegate.h"

void Add(int a, int b, int c)
{
	std::cout << a << " " << b << " " << c << std::endl;
}
void Add2(int a, int b, int c)
{
	std::cout << a << " " << b << " " << c << std::endl;
}

class A
{
public:
	void Add(int a, int b)
	{
		std::cout << a << " " << b << std::endl;
	}
};

class B : public A
{

};

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	//初始化COM
	if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
	{
		return -1;
	}

	AllocConsole();
	SetConsoleTitle(L"Output");
	FILE* tempFile = nullptr;
	freopen_s(&tempFile, "conin$", "r+t", stdin);
	freopen_s(&tempFile, "conout$", "w+t", stdout);

	Application* app = Application::Get();
	if (!app->Init(hInstance))
	{
		MessageBox(app->m_hWnd, L"窗口初始化失败!", L"Error", MB_OK | MB_ICONSTOP);
		return 0;
	}
	app->Run();

	//销毁COM
	CoUninitialize();

	return 1;
}