#include <windows.h>
#include <vector>

class IInputProcessor;
enum class EKeyState;

class Application
{
public:
	static Application* Get();

	bool Init(HINSTANCE hInstance, const wchar_t* title, int x, int y, int width, int height);
	bool Run();

	void Tick(float DeltaTime);

	void RegisterInputProcessor(IInputProcessor* InputProcessor, const int Index = -1);
	void UnRegisterInputProcessor(IInputProcessor* InputProcessor);

	EKeyState GetControlKeyState();
	EKeyState GetShiftKeyState();

	HWND hWnd;
	HINSTANCE hInstance;
private:
	Application();
	~Application() {}
	Application(const Application& other){};
	Application& operator =(const Application& other){};

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam); //消息处理函数
	bool RegisterWindow(HINSTANCE hInstance);
	bool CreateMyWindow(HINSTANCE hInstance, const wchar_t* title, int x, int y, int width, int height);

	static Application* app;
	std::vector<IInputProcessor*> InputProcessors;

	IInputProcessor* MainInputProcessor;

	bool bPressedControl = false;
	bool bPressedShift = false;
};