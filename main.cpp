#include <iostream>
#include "Engine/Math/vector.h"
#include "Engine/Math/matrix.h"
#include "Engine/Application/application.h"
#include <windows.h>

/*
int main()
{
	Vector2Type<float> a(10, 20);
	Vector3Type<float> b(1.f, 2.f, 3.f);
	Vector4Type<float> c(1.1, 2.2, 3.3);
	a.x = 10;
	a.y = 20;
	std::cout << a.x << "  " << a.y << std::endl;
	std::cout << b.x << "  " << b.y << "  " << b.z << std::endl;
	std::cout << c.x << "  " << c.y << "  " << c.z << "  " << c.w << std::endl;

	Matrix4x4f d;
	d[0][1] = 10;

	std:: cout << d[0][0] << " " << d[0][1] << std::endl;

	c = d.GetVector4(1);
	std::cout << c.x << "  " << c.y << "  " << c.z << "  " << c.w << std::endl;

	system("pause");
	return 0;
}
*/

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	Application* app = Application::Get();
	if (!app->Init(hInstance, __TEXT("Engine"), 100, 100, 800, 600))
	{
		MessageBox(app->hWnd, L"窗口初始化失败!", L"Error", MB_OK | MB_ICONSTOP);
		return 0;
	}
	app->Run();

	return 1;
}