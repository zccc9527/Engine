#pragma once
#include <iostream>
#include <windows.h>

#ifndef ThrowIfFailed

#define ThrowIfFailed(x)												\
{																		\
	HRESULT hr__ = x;													\
	if (FAILED(hr__))													\
	{																	\
		MessageBox(NULL, __FUNCTIONW__, L"Error", MB_OK | MB_ICONSTOP);	\
		exit(100);														\
	}																	\
}	
#endif // !ThrowIfFailed

#ifndef ThrowIfFailedM
#define ThrowIfFailedM(x, message)										\
{																		\
	HRESULT hr__ = x;													\
	if (FAILED(hr__))													\
	{																	\
		MessageBox(NULL, message, L"Error", MB_OK | MB_ICONSTOP);		\
		exit(100);														\
	}																	\
}	
#endif // !ThrowIfFailedM

#ifndef DXCall
#define DXCall(x)																\
if (FAILED(x))																	\
{																				\
	char line_number[32];														\
	sprintf_s(line_number, "%u", __LINE__);										\
	OutputDebugStringA("Error in: ");											\
	OutputDebugStringA(__FILE__);												\
	OutputDebugStringA("\nLine:");												\
	OutputDebugStringA(line_number);											\
	OutputDebugStringA("\n");													\
	OutputDebugStringA(#x);														\
	OutputDebugStringA("\n");													\
	__debugbreak();																\
}

#endif

#ifndef NAME_D3D12_OBJECT
#define NAME_D3D12_OBJECT(object, name) object->SetName(name); OutputDebugString(L"::D3D12 Object Created:  ");OutputDebugString(name);OutputDebugStringA("\n");
#endif