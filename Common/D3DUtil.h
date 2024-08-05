#ifndef D3D_UTIL_H
#define D3D_UTIL_H

#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <cassert>
#include <ctime>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

#include "d3dx11effect.h"
#include "DirectXTex.h"

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

class D3DHelper
{
public:
    static ID3D11ShaderResourceView* CreateTexture2DArraySRV(ID3D11Device* device,
                                                             ID3D11DeviceContext* context,
                                                             std::vector<std::wstring>& filenames,
                                                             DXGI_FORMAT format);
    static void LoadTexture2DFromDDSFile(ID3D11Device* device,
                                         const std::wstring& filename,
                                         DirectX::DDS_FLAGS flags,
                                         DirectX::TexMetadata* metadata,
                                         ID3D11ShaderResourceView** ppSrv);
    static ID3D11ShaderResourceView* CreateRandomTexture1DSRV(ID3D11Device* device);
};

// Order: left, right, bottom, top, near, far.
void ExtractFrustumPlanes(DirectX::XMFLOAT4 planes[6], DirectX::CXMMATRIX M);

#endif /* D3D_UTIL_H */