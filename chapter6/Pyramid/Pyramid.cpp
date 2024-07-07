#include "Pyramid.h"

Pyramid::Pyramid(HINSTANCE hInstance)
    : D3DApp(hInstance),
    mPyramidPosVertexBuffer(0),
    mPyramidColorVertexBuffer(0),
    mPyramidIndexBuffer(0),
    mFX(0),
    mTech(0),
    mfxWorldViewProj(0),
    mInputLayout(0),
    mPyramidIndexCount(0),
    mTheta(1.5f * MathHelper::Pi),
    mPhi(0.25f * MathHelper::Pi),
    mRadius(5.0f)
{
    mMainWndCaption = L"Pyramid";

    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    XMMATRIX I = XMMatrixIdentity();
    XMStoreFloat4x4(&mWorld, I);
    XMStoreFloat4x4(&mView, I);
    XMStoreFloat4x4(&mProj, I);
}

Pyramid::~Pyramid()
{
    ReleaseCOM(mPyramidPosVertexBuffer);
    ReleaseCOM(mPyramidColorVertexBuffer);
    ReleaseCOM(mPyramidIndexBuffer);
    ReleaseCOM(mFX);
    ReleaseCOM(mInputLayout);
}

bool Pyramid::Init()
{
    if (!D3DApp::Init())
        return false;

    BuildGeometryBuffers();
    BuildFX();
    BuildVertexLayout();

    D3D11_RASTERIZER_DESC rd;
    ZeroMemory(&rd, sizeof(D3D11_RASTERIZER_DESC));
    rd.FillMode = D3D11_FILL_WIREFRAME;
    rd.CullMode = D3D11_CULL_BACK;
    rd.FrontCounterClockwise = false;
    rd.DepthClipEnable = true;

    HR(md3dDevice->CreateRasterizerState(&rd, &mWireFrameRS));

    return true;
}

void Pyramid::OnResize()
{
    D3DApp::OnResize();

    // 창 크기가 재설정 되었으므로 원근 투영 행렬을 종횡비에 맞게 재설정.
    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj, P);
}

void Pyramid::UpdateScene(float dt)
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

void Pyramid::DrawScene()
{
    md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
    md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // 입력 어셈블러
    md3dImmediateContext->IASetInputLayout(mInputLayout);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 정점 & 인덱스 버퍼 설정
    UINT stride[] = { sizeof(XMFLOAT3), sizeof(XMFLOAT4) };
    UINT offset[] = { 0, 0 };

    ID3D11Buffer* vertexBuffer[] = { mPyramidPosVertexBuffer, mPyramidColorVertexBuffer };
    md3dImmediateContext->IASetVertexBuffers(0, 2, vertexBuffer, stride, offset);
    md3dImmediateContext->IASetIndexBuffer(mPyramidIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    md3dImmediateContext->RSSetState(mWireFrameRS);

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
        md3dImmediateContext->DrawIndexed(mPyramidIndexCount, 0, 0);
    }

    HR(mSwapChain->Present(0, 0));
}

void Pyramid::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void Pyramid::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void Pyramid::OnMouseMove(WPARAM btnState, int x, int y)
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

void Pyramid::BuildGeometryBuffers()
{
    // 정점 버퍼
    XMFLOAT3 verticesPos[] =
    {
        XMFLOAT3(+0.0f, +1.0f, +0.0f), // 중심 정점
        XMFLOAT3(+1.0f, -1.0f, +1.0f), // 우 하단 정점 (앞)
        XMFLOAT3(-1.0f, -1.0f, +1.0f), // 좌 하단 정점 (앞)
        XMFLOAT3(+1.0f, -1.0f, -1.0f), // 우 하단 정점 (뒤)
        XMFLOAT3(-1.0f, -1.0f, -1.0f), // 좌 하단 정점 (뒤)
    };

    XMFLOAT4 verticesColor[] =
    {
        XMFLOAT4(Colors::White),
        XMFLOAT4(Colors::Black),
        XMFLOAT4(Colors::Red),
        XMFLOAT4(Colors::Green),
        XMFLOAT4(Colors::Blue),
    };

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(verticesPos);
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    vbd.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA vpInitData;
    vpInitData.pSysMem = verticesPos;

    HR(md3dDevice->CreateBuffer(&vbd, &vpInitData, &mPyramidPosVertexBuffer));

    D3D11_BUFFER_DESC vcbd;
    vcbd.Usage = D3D11_USAGE_IMMUTABLE;
    vcbd.ByteWidth = sizeof(verticesColor);
    vcbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vcbd.CPUAccessFlags = 0;
    vcbd.MiscFlags = 0;
    vcbd.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA vcInitData;
    vcInitData.pSysMem = verticesColor;

    HR(md3dDevice->CreateBuffer(&vcbd, &vcInitData, &mPyramidColorVertexBuffer));

    // 인덱스 버퍼
    /*UINT indices[] =
    {
        0, 2, 1, 
        0, 4, 2,
        0, 1, 3,
        0, 3, 4,
        4, 1, 2,
        4, 3, 1
    };
    */
    UINT indices[] =
    {
        1, 0, 2,
        2, 0, 4,
        3, 0, 1,
        4, 0, 3,
        2, 4, 1,
        1, 4, 3
    };


    mPyramidIndexCount = sizeof(indices);

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = mPyramidIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    ibd.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA iInitData;
    iInitData.pSysMem = indices;

    HR(md3dDevice->CreateBuffer(&ibd, &iInitData, &mPyramidIndexBuffer));
}

void Pyramid::BuildFX()
{
    std::ifstream fin("shaders/color.cso", std::ios::binary);

    fin.seekg(0, std::ios::end);
    int size = (int)fin.tellg();
    fin.seekg(0, std::ios::beg);
    std::vector<char> compiledShader(size);

    fin.read(&compiledShader[0], size);
    fin.close();

    HR(D3DX11CreateEffectFromMemory(&compiledShader[0], size, 0, md3dDevice, &mFX));
    
    mTech            = mFX->GetTechniqueByName("ColorTech");
    mfxWorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
}

void Pyramid::BuildVertexLayout()
{
    D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    D3DX11_PASS_DESC passDesc;
    mTech->GetPassByIndex(0)->GetDesc(&passDesc);
    HR(md3dDevice->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mInputLayout));
}