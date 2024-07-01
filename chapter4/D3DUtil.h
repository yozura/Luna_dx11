#ifndef D3DUTIL_H
#define D3DUTIL_H

#include <d3d11.h>
#include <dxgi.h>
#include <cassert>
#include <ctime>
#include <fstream>
#include <sstream>

// HRESULT ���� �ڵ鷯
#if defined(DEBUG) | defined(_DEBUG)
	#ifndef HR
	#define HR(x)\
	{\
		HRESULT hr = (x);\
		if (FAILED(hr))\
		{\
			LPWSTR output;\
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &output, 0, NULL);\
			MessageBox(NULL, output, TEXT("ERROR"), MB_OK);\
		}\
	}
	#endif
#else
	#ifndef HR
	#define HR(x) (x)
	#endif
#endif

// COM ��ü ���� ��ũ��
#define ReleaseCOM(x) { if (x) { x->Release(); x = 0; } }

// �Ϲ� ��ü ���� ��ũ��
#define SafeDelete(x) { delete x; x = 0; }

#endif /* D3DUTIL_H */