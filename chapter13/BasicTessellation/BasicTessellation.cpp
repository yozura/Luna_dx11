#include "BasicTessellation.h"

using namespace DirectX;

BasicTessellation::BasicTessellation(HINSTANCE hInstance)
    : D3DApp(hInstance), mQuadPatchVB(0), mEyePosW(0.0f, 0.0f, 0.0f)
    , mTheta(1.3f * MathHelper::Pi), mPhi(0.2f * MathHelper::Pi), mRadius(80.0f)
{
    mMainWndCaption = L"Basic Tessellation";
    mEnable4xMsaa = false;

    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    XMMATRIX I = XMMatrixIdentity();
    XMStoreFloat4x4(&mView, I);
    XMStoreFloat4x4(&mProj, I);
}

BasicTessellation::~BasicTessellation()
{
    md3dImmediateContext->ClearState();
    ReleaseCOM(mQuadPatchVB);

    Effects::DestroyAll();
    InputLayouts::DestroyAll();
    RenderStates::DestroyAll();
}

bool BasicTessellation::Init()
{
    if (!D3DApp::Init())
        return false;

    Effects::InitAll(md3dDevice);
    InputLayouts::InitAll(md3dDevice);
    RenderStates::InitAll(md3dDevice);

    BuildQuadPatchBuffer();

    return true;
}

void BasicTessellation::OnResize()
{
    D3DApp::OnResize();

    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj, P);
}

void BasicTessellation::UpdateScene(float dt)
{
    float x = mRadius * sinf(mPhi) * cosf(mTheta);
    float z = mRadius * sinf(mPhi) * sinf(mTheta);
    float y = mRadius * cosf(mPhi);

    mEyePosW = XMFLOAT3(x, y, z);

    XMVECTOR pos    = XMVectorSet(x, y, z, 0.0f);
    XMVECTOR target = XMVectorZero();
    XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
    XMStoreFloat4x4(&mView, V);
}

void BasicTessellation::DrawScene()
{
    md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
    md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

    XMMATRIX view = XMLoadFloat4x4(&mView);
    XMMATRIX proj = XMLoadFloat4x4(&mProj);

    md3dImmediateContext->IASetInputLayout(InputLayouts::Pos);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

    UINT stride = sizeof(Vertex::Pos);
    UINT offset = 0;

    Effects::TessellationFX->SetEyePosW(mEyePosW);
    Effects::TessellationFX->SetFogColor(Colors::Silver);
    Effects::TessellationFX->SetFogStart(15.0f);
    Effects::TessellationFX->SetFogRange(175.0f);

    D3DX11_TECHNIQUE_DESC techDesc;
    Effects::TessellationFX->TessTech->GetDesc(&techDesc);

    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        md3dImmediateContext->IASetVertexBuffers(0, 1, &mQuadPatchVB, &stride, &offset);

        XMMATRIX world = XMMatrixIdentity();
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
        XMMATRIX worldViewProj = world * view * proj;
        
        Effects::TessellationFX->SetWorld(world);
        Effects::TessellationFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::TessellationFX->SetWorldViewProj(worldViewProj);
        Effects::TessellationFX->SetTexTransform(XMMatrixIdentity());
        Effects::TessellationFX->SetDiffuseMap(0);

        Effects::TessellationFX->TessTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        
        md3dImmediateContext->RSSetState(RenderStates::WireFrameRS);
        md3dImmediateContext->Draw(4, 0);
    }

    HR(mSwapChain->Present(0, 0));
}


void BasicTessellation::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void BasicTessellation::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void BasicTessellation::OnMouseMove(WPARAM btnState, int x, int y)
{
    if ((btnState & MK_LBUTTON) != 0)
    {
        // Make each pixel correspond to a quarter of a degree.
        float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
        float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

        // Update angles based on input to orbit camera around box.
        mTheta += dx;
        mPhi += dy;

        // Restrict the angle mPhi.
        mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
    }
    else if ((btnState & MK_RBUTTON) != 0)
    {
        float dx = 0.2f * static_cast<float>(x - mLastMousePos.x);
        float dy = 0.2f * static_cast<float>(y - mLastMousePos.y);

        // Update the camera radius based on input.
        mRadius += dx - dy;

        // Restrict the radius.
        mRadius = MathHelper::Clamp(mRadius, 5.0f, 300.0f);
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}

void BasicTessellation::BuildQuadPatchBuffer()
{
    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(XMFLOAT3) * 4;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;

    XMFLOAT3 vertices[4] =
    {
        XMFLOAT3(-10.0f, 0.0f, +10.0f),
        XMFLOAT3(+10.0f, 0.0f, +10.0f),
        XMFLOAT3(-10.0f, 0.0f, -10.0f),
        XMFLOAT3(+10.0f, 0.0f, -10.0f)
    };

    D3D11_SUBRESOURCE_DATA vInitData;
    vInitData.pSysMem = &vertices[0];

    HR(md3dDevice->CreateBuffer(&vbd, &vInitData, &mQuadPatchVB));
}
