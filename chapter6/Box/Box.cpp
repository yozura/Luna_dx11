#include "Box.h"

Box::Box(HINSTANCE hInstance)
    : D3DApp(hInstance),
    mBoxVB(0),
    mBoxIB(0),
    mFX(0),
    mTech(0),
    mfxWorldViewProj(0),
    mInputLayout(0),
    mTheta(1.5f * MathHelper::Pi),
    mPhi(0.25f * MathHelper::Pi),
    mRadius(5.0f)
{
    mMainWndCaption = L"Box";

    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    XMMATRIX I = XMMatrixIdentity();
    XMStoreFloat4x4(&mWorld, I);
    XMStoreFloat4x4(&mView, I);
    XMStoreFloat4x4(&mProj, I);
}

Box::~Box()
{
    ReleaseCOM(mBoxVB);
    ReleaseCOM(mBoxIB);
    ReleaseCOM(mFX);
    ReleaseCOM(mInputLayout);
}

bool Box::Init()
{
    if (!D3DApp::Init())
        return false;

    BuildGeometryBuffers();
    BuildFX();
    BuildVertexLayout();

    return true;
}

void Box::OnResize()
{
    D3DApp::OnResize();

    // 창 크기가 재설정 되었으므로 원근 투영 행렬을 종횡비에 맞게 재설정.
    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj, P);
}

void Box::UpdateScene(float dt)
{
    // 구면 좌표를 직교 좌표로 변환
    float x = mRadius * sinf(mPhi) * cosf(mTheta);
    float z = mRadius * sinf(mPhi) * sinf(mTheta);
    float y = mRadius * cosf(mPhi);

    // 뷰 행렬 설정
    XMVECTOR pos    = XMVectorSet(x, y, z, 1.0f);
    XMVECTOR target = XMVectorZero();
    XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
    XMStoreFloat4x4(&mView, V);
}

void Box::DrawScene()
{
    md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
    md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // 입력 어셈블러
    md3dImmediateContext->IASetInputLayout(mInputLayout);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 정점 & 인덱스 버퍼 설정
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
    md3dImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);

    // 상수 버퍼 설정
    XMMATRIX world = XMLoadFloat4x4(&mWorld);
    XMMATRIX view  = XMLoadFloat4x4(&mView);
    XMMATRIX proj  = XMLoadFloat4x4(&mProj);
    XMMATRIX worldViewProj = world * view * proj;

    mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
    
    D3DX11_TECHNIQUE_DESC techDesc;
    mTech->GetDesc(&techDesc);

    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexed(36, 0, 0);
    }

    HR(mSwapChain->Present(0, 0));
}

void Box::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void Box::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void Box::OnMouseMove(WPARAM btnState, int x, int y)
{
    if ((btnState & MK_LBUTTON) != 0)
    {
        float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
        float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

        mTheta += dx;
        mPhi   += dy;

        mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
    }
    else if ((btnState & MK_RBUTTON) != 0)
    {
        float dx = 0.005f * static_cast<float>(x - mLastMousePos.x);
        float dy = 0.005f * static_cast<float>(y - mLastMousePos.y);

        mRadius += dx - dy;

        mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}

void Box::BuildGeometryBuffers()
{
    // 정점 버퍼
    Vertex vertices[] =
    {
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White)   },
        { XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Black)   },
        { XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red)     },
        { XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green)   },
        { XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue)    },
        { XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Yellow)  },
        { XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan)    },
        { XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta) }
    };

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex) * 8;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    vbd.StructureByteStride = 0;
    
    D3D11_SUBRESOURCE_DATA vInitData;
    vInitData.pSysMem = vertices;
    
    HR(md3dDevice->CreateBuffer(&vbd, &vInitData, &mBoxVB));

    // 인덱스 버퍼
    UINT indices[] =
    {
        // 앞
        0, 1, 2,
        0, 2, 3,

        // 뒤
        4, 6, 5,
        4, 7, 6,

        // 왼쪽
        4, 5, 1,
        4, 1, 0,

        // 오른쪽
        3, 2, 6,
        3, 6, 7,

        // 위
        1, 5, 6,
        1, 6, 2,

        // 아래
        4, 0, 3,
        4, 3, 7
    };

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * 36;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    ibd.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA iInitData;
    iInitData.pSysMem = indices;
    
    HR(md3dDevice->CreateBuffer(&ibd, &iInitData, &mBoxIB));
}

void Box::BuildFX()
{
    DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
    shaderFlags |= D3D10_SHADER_DEBUG;
    shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
    
    ID3D10Blob* compiledShader = 0;
    ID3D10Blob* compilationMsgs = 0;
    HRESULT hr = D3DCompileFromFile(
        L"shaders/color.fx",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "fx_5_0",
        "fx_5_0",
        shaderFlags,
        0,
        &compiledShader,
        &compilationMsgs
    );

    if (compilationMsgs != 0)
    {
        MessageBoxA(0, (char*)compilationMsgs->GetBufferPointer(), 0, 0);
        ReleaseCOM(compilationMsgs);
    }

    if (FAILED(hr))
    {
        LPWSTR output;
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&output, 0, NULL);
        MessageBox(NULL, output, TEXT("Error: D3DCompileFromFile"), MB_OK);
    }

    HR(D3DX11CreateEffectFromMemory(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), 0, md3dDevice, &mFX));
    ReleaseCOM(compiledShader);

    mTech            = mFX->GetTechniqueByName("ColorTech");
    mfxWorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
}

void Box::BuildVertexLayout()
{
    D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    D3DX11_PASS_DESC passDesc;
    mTech->GetPassByIndex(0)->GetDesc(&passDesc);
    HR(md3dDevice->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mInputLayout));
}