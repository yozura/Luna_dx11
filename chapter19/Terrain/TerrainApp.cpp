#include "TerrainApp.h"

using namespace DirectX;

TerrainApp::TerrainApp(HINSTANCE hInstance)
    : D3DApp(hInstance), mSky(0), mWalkCamMode(false)
{
    mMainWndCaption = L"Terrain";
    mEnable4xMsaa = false;

    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    mCam.SetPosition(0.0f, 2.0f, 100.0f);

    mDirLights[0].Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    mDirLights[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mDirLights[0].Specular = XMFLOAT4(0.8f, 0.8f, 0.7f, 1.0f);
    mDirLights[0].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

    mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mDirLights[1].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirLights[1].Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirLights[1].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

    mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mDirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirLights[2].Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirLights[2].Direction = XMFLOAT3(-0.57735f, -0.57735f, -0.57735f);
}

TerrainApp::~TerrainApp()
{
    md3dImmediateContext->ClearState();

    SafeDelete(mSky);

    Effects::DestroyAll();
    InputLayouts::DestroyAll();
    RenderStates::DestroyAll();
}

bool TerrainApp::Init()
{
    if (!D3DApp::Init())
        return false;

    // Must init Effects first since InputLayouts depend on shader signatures.
    Effects::InitAll(md3dDevice);
    InputLayouts::InitAll(md3dDevice);
    RenderStates::InitAll(md3dDevice);

    mSky = new Sky(md3dDevice, L"Textures/grasscube1024.dds", 5000.0f);

    Terrain::InitInfo tii;
    tii.HeightMapFilename = L"textures/terrain.raw";
    tii.LayerMapFilename0 = L"textures/grass.dds";
    tii.LayerMapFilename1 = L"textures/darkdirt.dds";
    tii.LayerMapFilename2 = L"textures/stone.dds";
    tii.LayerMapFilename3 = L"textures/lightdirt.dds";
    tii.LayerMapFilename4 = L"textures/snow.dds";
    tii.BlendMapFilename = L"textures/blend.dds";
    tii.HeightScale = 50.0f;
    tii.HeightMapWidth = 2049;
    tii.HeightMapHeight = 2049;
    tii.CellSpacing = 0.5f;

    mTerrain.Init(md3dDevice, md3dImmediateContext, tii);
    return true;
}

void TerrainApp::OnResize()
{
    D3DApp::OnResize();

    mCam.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 3000.0f);
}

void TerrainApp::UpdateScene(float dt)
{
    // Control the camera.
    if (GetAsyncKeyState('W') & 0x8000)
        mCam.Walk(10.0f * dt);
    if (GetAsyncKeyState('S') & 0x8000)
        mCam.Walk(-10.0f * dt);
    if (GetAsyncKeyState('A') & 0x8000)
        mCam.Strafe(-10.0f * dt);
    if (GetAsyncKeyState('D') & 0x8000)
        mCam.Strafe(10.0f * dt);

    // Walk/fly mode
    if (GetAsyncKeyState('2') & 0x8000)
        mWalkCamMode = true;
    if (GetAsyncKeyState('3') & 0x8000)
        mWalkCamMode = false;

    // Clamp camera to terrain surface in walk mode.
    if (mWalkCamMode)
    {
        XMFLOAT3 camPos = mCam.GetPosition();
        float y = mTerrain.GetHeight(camPos.x, camPos.z);
        mCam.SetPosition(camPos.x, y + 2.0f, camPos.z);
    }

    mCam.UpdateViewMatrix();
}

void TerrainApp::DrawScene()
{
    md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
    md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

    if (GetAsyncKeyState('1') & 0x8000)
        md3dImmediateContext->RSSetState(RenderStates::WireFrameRS);

    mTerrain.Draw(md3dImmediateContext, mCam, mDirLights);

    md3dImmediateContext->RSSetState(0);

    mSky->Draw(md3dImmediateContext, mCam);

    // restore default states, as the SkyFX changes them in the effect file.
    md3dImmediateContext->RSSetState(0);
    md3dImmediateContext->OMSetDepthStencilState(0, 0);

    HR(mSwapChain->Present(0, 0));
}

void TerrainApp::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void TerrainApp::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void TerrainApp::OnMouseMove(WPARAM btnState, int x, int y)
{
    if ((btnState & MK_LBUTTON) != 0)
    {
        // Make each pixel correspond to a quarter of a degree.
        float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
        float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

        mCam.Pitch(dy);
        mCam.RotateY(dx);
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}
