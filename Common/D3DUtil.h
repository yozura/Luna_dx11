#ifndef D3D_UTIL_H
#define D3D_UTIL_H

#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <DirectXColors.h>
#include <cassert>
#include <ctime>
#include <fstream>
#include <sstream>
#include <vector>

#include "d3dx11effect.h"
#include "MathHelper.h"

// HRESULT 에러 핸들러
#if defined(DEBUG) | defined(_DEBUG)
	#ifndef HR
	#define HR(x)\
	{\
		HRESULT hr = (x);\
		if (FAILED(hr))\
		{\
			LPWSTR output;\
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,\
						  NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &output, 0, NULL);\
			MessageBox(NULL, output, TEXT("ERROR"), MB_OK);\
		}\
	}
	#endif
#else
	#ifndef HR
	#define HR(x) (x)
	#endif
#endif

// COM 객체 해제 매크로
#define ReleaseCOM(x) { if (x) { x->Release(); x = 0; } }

// 일반 객체 해제 매크로
#define SafeDelete(x) { delete x; x = 0; }

#endif /* D3D_UTIL_H */