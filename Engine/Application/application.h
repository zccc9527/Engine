#include <windows.h>
#include <vector>
#include <d2d1.h>
#include "../Math/vector.h"
#include <string>
#include <d3d12.h>
#pragma comment(lib, "d3d12.lib")
#include <dxgi1_6.h>
#pragma comment(lib, "dxgi.lib")
#include <wrl.h>
using Microsoft::WRL::ComPtr;

#include <DirectXMath.h>
#include <DirectXColors.h>
#include <D3Dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

class IInputProcessor;
enum class EKeyState;

class Application
{
public:
	static Application* Get();

	bool Init(HINSTANCE hInstance);
	bool InitWindow(HINSTANCE hInstance);
	bool InitDirectX();

	void OnResize();

	bool Run();

	void Tick(float DeltaTime);

	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam); //消息处理函数

	void RegisterInputProcessor(IInputProcessor* InputProcessor, const int Index = -1);
	void UnRegisterInputProcessor(IInputProcessor* InputProcessor);

	EKeyState GetControlKeyState();
	EKeyState GetShiftKeyState();

	HRESULT DrawLine(Vector2f Start, Vector2f End, Color c, float width = 1.0f);

	HWND m_hWnd;
	HINSTANCE hInstance;
private:
	Application(int width, int height);
	~Application() {}
	Application(const Application& other){};
	Application& operator =(const Application& other){};

	
	bool RegisterWindow(HINSTANCE hInstance);
	bool CreateMyWindow(HINSTANCE hInstance);

	template<typename T>
	void SafeRelease(T** ppInterfaceToRelease);

	HRESULT CreateGraphicsResource(HWND hWnd);
	void ReleaseAllResource();

	void OnPaint(HWND hWnd);

	void CreateDevice();
	void CreateFence();
	void GetDescriptorSize();
	void SetMSAA();
	void CreateCommandObject();
	void CreateSwapChain();
	void CreateDescriptorHeap();
	void CreateRTV();
	void CreateDSV();
	void FlushCommandQueue();
	void CreateViewportAndScissorRect(); //创建视口和裁剪矩形

	void Draw();
	void CalculateFrameState();

	static Application* app;
	std::vector<IInputProcessor*> InputProcessors;

	IInputProcessor* MainInputProcessor;

	bool bPressedControl = false;
	bool bPressedShift = false;

	int m_width;
	int m_height;

	ID2D1Factory* pD2DFactory = nullptr; //d2d工厂
	ID2D1HwndRenderTarget* pRenderTarget; //d2d绘制接口

	ComPtr<IDXGIFactory4> pDXGIFactory;
	ComPtr<ID3D12Device> pDevice;
	ComPtr<ID3D12Fence> pFence;

	UINT rtvDescriptorSize; //render target view 描述符大小
	UINT dsvDescriptorSize; //depth stencil view 描述符大小
	UINT cbv_srv_uavDescriptorSize; //常亮缓冲视图、着色器资源视图、随机访问视图，constant buffer view、shader resource view、unordered access view 描述符大小

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaQualityLevels;

	ComPtr<ID3D12CommandQueue> pCommandQueue;
	ComPtr<ID3D12CommandAllocator> pCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> pCommandList;

	ComPtr<IDXGISwapChain> pSwapChain;

	ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
	ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;

	ComPtr<ID3D12Resource> swapChainBuffer[2];
	ComPtr<ID3D12Resource> depthStencilBuffer;

	UINT mCurrentFence = 0;

	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;

	UINT mCurrentBackBuffer = 0;

	class GameTime* gt = nullptr; //计时类
	int mFrameCount = 0; //总帧数
	double timeElapsed = 0.f; //流逝的时间

	bool bMinimized = false;
	bool bMaximized = false;
	bool bResizing = false;
};

