#pragma once
#include <iostream>

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

