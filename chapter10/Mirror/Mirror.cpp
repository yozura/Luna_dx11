#include "Mirror.h"

using namespace DirectX;

Mirror::Mirror(HINSTANCE hInstance)
    : D3DApp(hInstance)
    , mRoomVertexBuffer(0), mSkullVertexBuffer(0), mSkullIndexBuffer(0), mSkullTranslation(0.0f, 1.0f, -5.0f)
    , mFloorDiffuseMapSRV(0), mWallDiffuseMapSRV(0), mMirrorDiffuseMapSRV(0), mEyePosW(0.0f, 0.0f, 0.0f)
    , mRenderOptions(RenderOptions::Textures), mTheta(1.24f * MathHelper::Pi), mPhi(0.42f * MathHelper::Pi)
    , mRadius(12.0f)
{
    mMainWndCaption = L"Mirror";
    mEnable4xMsaa = false;

    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    XMMATRIX I = XMMatrixIdentity();
    XMStoreFloat4x4(&mRoomWorld, I);
    XMStoreFloat4x4(&mView, I);
    XMStoreFloat4x4(&mProj, I);

    mDirLights[0].Ambient   = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirLights[0].Diffuse   = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    mDirLights[0].Specular  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    mDirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

    mDirLights[1].Ambient   = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mDirLights[1].Diffuse   = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
    mDirLights[1].Specular  = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
    mDirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

    mDirLights[2].Ambient   = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mDirLights[2].Diffuse   = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirLights[2].Specular  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mDirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

    mRoomMat.Ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    mRoomMat.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mRoomMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

    mSkullMat.Ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    mSkullMat.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mSkullMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

    // Reflected material is transparent so it blends into mirror.
    mMirrorMat.Ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    mMirrorMat.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
    mMirrorMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

    mShadowMat.Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mShadowMat.Diffuse  = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f);
    mShadowMat.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);
}

Mirror::~Mirror()
{
    md3dImmediateContext->ClearState();

    ReleaseCOM(mRoomVertexBuffer);
    ReleaseCOM(mSkullVertexBuffer);
    ReleaseCOM(mSkullIndexBuffer);

    ReleaseCOM(mFloorDiffuseMapSRV);
    ReleaseCOM(mWallDiffuseMapSRV);
    ReleaseCOM(mMirrorDiffuseMapSRV);

    Effects::DestroyAll();
    InputLayouts::DestroyAll();
    RenderStates::DestroyAll();
}

bool Mirror::Init()
{
    if (!D3DApp::Init())
        return false;

    Effects::InitAll(md3dDevice);
    InputLayouts::InitAll(md3dDevice);
    RenderStates::InitAll(md3dDevice);

    // Load Textures
    CreateShaderResourceViewFromFile();
    
    BuildRoomGeometryBuffers();
    BuildSkullGeometryBuffers();

    return true;
}

void Mirror::OnResize()
{
    D3DApp::OnResize();

    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj, P);
}

void Mirror::UpdateScene(float dt)
{
    float x = mRadius * sinf(mPhi) * cosf(mTheta);
    float z = mRadius * sinf(mPhi) * sinf(mTheta);
    float y = mRadius * cosf(mPhi);

    mEyePosW = XMFLOAT3(x, y, z);

    XMVECTOR pos    = XMVectorSet(x, y, z, 1.0f);
    XMVECTOR target = XMVectorZero();
    XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
    XMStoreFloat4x4(&mView, V);

    // Switch the render mode base in key input.
    if (GetAsyncKeyState('1') & 0x8000)
        mRenderOptions = RenderOptions::Lighting;
    if (GetAsyncKeyState('2') & 0x8000)
        mRenderOptions = RenderOptions::Textures;
    if (GetAsyncKeyState('3') & 0x8000)
        mRenderOptions = RenderOptions::TexturesAndFog;

    // Allow user to move box
    if (GetAsyncKeyState('W') & 0x8000)
        mSkullTranslation.y += 1.0f * dt;
    if (GetAsyncKeyState('S') & 0x8000)
        mSkullTranslation.y -= 1.0f * dt;
    if (GetAsyncKeyState('A') & 0x8000)
        mSkullTranslation.x -= 1.0f * dt;
    if (GetAsyncKeyState('D') & 0x8000)
        mSkullTranslation.x += 1.0f * dt;
    
    mSkullTranslation.y = MathHelper::Max(mSkullTranslation.y, 0.0f);

    XMMATRIX skullRotate = XMMatrixRotationY(0.5f * MathHelper::Pi);
    XMMATRIX skullScale  = XMMatrixScaling(0.45f, 0.45f, 0.45f);
    XMMATRIX skullOffset = XMMatrixTranslation(mSkullTranslation.x, mSkullTranslation.y, mSkullTranslation.z);
    XMStoreFloat4x4(&mSkullWorld, skullRotate * skullScale * skullOffset);
}

void Mirror::DrawScene()
{
    md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Black));
    md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

    UINT stride = sizeof(Vertex::Basic32);
    UINT offset = 0;

    XMMATRIX view = XMLoadFloat4x4(&mView);
    XMMATRIX proj = XMLoadFloat4x4(&mProj);
    
    Effects::BasicFX->SetDirLights(mDirLights);
    Effects::BasicFX->SetEyePosW(mEyePosW);
    Effects::BasicFX->SetFogColor(Colors::Black);
    Effects::BasicFX->SetFogStart(2.0f);
    Effects::BasicFX->SetFogRange(40.0f);
    
    ID3DX11EffectTechnique* activeTech      = 0;
    ID3DX11EffectTechnique* activeSkullTech = 0;

    switch (mRenderOptions)
    {
    case RenderOptions::Lighting:
        activeTech      = Effects::BasicFX->Light3Tech;
        activeSkullTech = Effects::BasicFX->Light3Tech;
        break;
    case RenderOptions::Textures:
        activeTech      = Effects::BasicFX->Light3TexTech;
        activeSkullTech = Effects::BasicFX->Light3Tech;
        break;
    case RenderOptions::TexturesAndFog:
        activeTech      = Effects::BasicFX->Light3TexFogTech;
        activeSkullTech = Effects::BasicFX->Light3FogTech;
        break;
    }

    D3DX11_TECHNIQUE_DESC techDesc;
    
    activeTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        ID3DX11EffectPass* pass = activeTech->GetPassByIndex(p);

        md3dImmediateContext->IASetVertexBuffers(0, 1, &mRoomVertexBuffer, &stride, &offset);

        XMMATRIX world = XMLoadFloat4x4(&mRoomWorld);
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
        XMMATRIX worldViewProj = world * view * proj;

        Effects::BasicFX->SetWorld(world);
        Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::BasicFX->SetWorldViewProj(worldViewProj);
        Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
        Effects::BasicFX->SetMaterial(mRoomMat);

        // Floor
        Effects::BasicFX->SetDiffuseMap(mFloorDiffuseMapSRV);
        pass->Apply(0, md3dImmediateContext);
        md3dImmediateContext->Draw(6, 0);

        // Wall
        Effects::BasicFX->SetDiffuseMap(mWallDiffuseMapSRV);
        pass->Apply(0, md3dImmediateContext);
        md3dImmediateContext->Draw(18, 6);
    }

    activeSkullTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        ID3DX11EffectPass* pass = activeSkullTech->GetPassByIndex(p);

        md3dImmediateContext->IASetVertexBuffers(0, 1, &mSkullVertexBuffer, &stride, &offset);
        md3dImmediateContext->IASetIndexBuffer(mSkullIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

        // Skull
        XMMATRIX world = XMLoadFloat4x4(&mSkullWorld);
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
        XMMATRIX worldViewProj = world * view * proj;

        Effects::BasicFX->SetWorld(world);
        Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::BasicFX->SetWorldViewProj(worldViewProj);
        Effects::BasicFX->SetMaterial(mSkullMat);

        pass->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);
    }

    activeTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        ID3DX11EffectPass* pass = activeTech->GetPassByIndex(p);

        md3dImmediateContext->IASetVertexBuffers(0, 1, &mRoomVertexBuffer, &stride, &offset);

        // Mirror
        XMMATRIX world = XMLoadFloat4x4(&mRoomWorld);
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
        XMMATRIX worldViewProj = world * view * proj;

        Effects::BasicFX->SetWorld(world);
        Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::BasicFX->SetWorldViewProj(worldViewProj);
        Effects::BasicFX->SetTexTransform(XMMatrixIdentity());

        md3dImmediateContext->OMSetBlendState(RenderStates::NoRenderTargetWriteBS, blendFactor, 0xffffffff);
        md3dImmediateContext->OMSetDepthStencilState(RenderStates::MarkMirrorDSS, 1);

        pass->Apply(0, md3dImmediateContext);
        md3dImmediateContext->Draw(6, 24);

        // Restore States.
        md3dImmediateContext->OMSetDepthStencilState(0, 0);
        md3dImmediateContext->OMSetBlendState(0, blendFactor, 0xfffffff);
    }

    activeSkullTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        ID3DX11EffectPass* pass = activeSkullTech->GetPassByIndex(p);

        md3dImmediateContext->IASetVertexBuffers(0, 1, &mSkullVertexBuffer, &stride, &offset);
        md3dImmediateContext->IASetIndexBuffer(mSkullIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

        // Skull
        XMVECTOR mirrorPlane       = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
        XMMATRIX R                 = XMMatrixReflect(mirrorPlane);
        XMMATRIX world             = XMLoadFloat4x4(&mSkullWorld) * R;
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
        XMMATRIX worldViewProj     = world * view * proj;

        Effects::BasicFX->SetWorld(world);
        Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::BasicFX->SetWorldViewProj(worldViewProj);
        Effects::BasicFX->SetMaterial(mSkullMat);

        XMFLOAT3 oldLightDirections[3];
        for (int i = 0; i < 3; ++i)
        {
            oldLightDirections[i] = mDirLights[i].Direction;

            XMVECTOR lightDir = XMLoadFloat3(&mDirLights[i].Direction);
            XMVECTOR reflectedLightDir = XMVector3TransformNormal(lightDir, R);
            XMStoreFloat3(&mDirLights[i].Direction, reflectedLightDir);
        }

        Effects::BasicFX->SetDirLights(mDirLights);

        md3dImmediateContext->RSSetState(RenderStates::CullClockwiseRS);
        md3dImmediateContext->OMSetDepthStencilState(RenderStates::DrawReflectDSS, 1);

        pass->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);

        md3dImmediateContext->RSSetState(0);
        md3dImmediateContext->OMSetDepthStencilState(0, 0);

        for (int i = 0; i < 3; ++i)
        {
            mDirLights[i].Direction = oldLightDirections[i];
        }

        Effects::BasicFX->SetDirLights(mDirLights);
    }

    activeTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        ID3DX11EffectPass* pass = activeTech->GetPassByIndex(p);

        md3dImmediateContext->IASetVertexBuffers(0, 1, &mRoomVertexBuffer, &stride, &offset);

        XMMATRIX world = XMLoadFloat4x4(&mRoomWorld);
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
        XMMATRIX worldViewProj = world * view * proj;

        Effects::BasicFX->SetWorld(world);
        Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::BasicFX->SetWorldViewProj(worldViewProj);
        Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
        Effects::BasicFX->SetMaterial(mMirrorMat);
        Effects::BasicFX->SetDiffuseMap(mMirrorDiffuseMapSRV);

        // Mirror
        md3dImmediateContext->OMSetBlendState(RenderStates::TransparentBS, blendFactor, 0xfffffff);
        pass->Apply(0, md3dImmediateContext);
        md3dImmediateContext->Draw(6, 24);
    }

    activeSkullTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        ID3DX11EffectPass* pass = activeSkullTech->GetPassByIndex(p);

        md3dImmediateContext->IASetVertexBuffers(0, 1, &mSkullVertexBuffer, &stride, &offset);
        md3dImmediateContext->IASetIndexBuffer(mSkullIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

        XMVECTOR shadowPlane = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); 
        XMVECTOR toMainLight = -XMLoadFloat3(&mDirLights[0].Direction);
        XMMATRIX S = XMMatrixShadow(shadowPlane, toMainLight);
        XMMATRIX shadowOffsetY = XMMatrixTranslation(0.0f, 0.001f, 0.0f);

        XMMATRIX world = XMLoadFloat4x4(&mSkullWorld) * S * shadowOffsetY;
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
        XMMATRIX worldViewProj = world * view * proj;

        Effects::BasicFX->SetWorld(world);
        Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::BasicFX->SetWorldViewProj(worldViewProj);
        Effects::BasicFX->SetMaterial(mShadowMat);

        md3dImmediateContext->OMSetDepthStencilState(RenderStates::NoDoubleBlendDSS, 0);
        pass->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);

        md3dImmediateContext->OMSetBlendState(0, blendFactor, 0xffffffff);
        md3dImmediateContext->OMSetDepthStencilState(0, 0);
    }

    HR(mSwapChain->Present(0, 0));
}


void Mirror::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void Mirror::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void Mirror::OnMouseMove(WPARAM btnState, int x, int y)
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
        // Make each pixel correspond to 0.01 unit in the scene.
        float dx = 0.01f * static_cast<float>(x - mLastMousePos.x);
        float dy = 0.01f * static_cast<float>(y - mLastMousePos.y);

        // Update the camera radius based on input.
        mRadius += dx - dy;

        // Restrict the radius.
        mRadius = MathHelper::Clamp(mRadius, 3.0f, 50.0f);
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}

void Mirror::CreateShaderResourceViewFromFile()
{
    ScratchImage floor;
    HR(LoadFromDDSFile(L"textures/checkboard.dds", DDS_FLAGS_NONE, nullptr, floor));
    HR(CreateShaderResourceView(md3dDevice, floor.GetImages(), floor.GetImageCount(), floor.GetMetadata(), &mFloorDiffuseMapSRV));

    ScratchImage wall;
    HR(LoadFromDDSFile(L"textures/brick01.dds", DDS_FLAGS_NONE, nullptr, wall));
    HR(CreateShaderResourceView(md3dDevice, wall.GetImages(), wall.GetImageCount(), wall.GetMetadata(), &mWallDiffuseMapSRV));

    ScratchImage mirror;
    HR(LoadFromDDSFile(L"textures/ice.dds", DDS_FLAGS_NONE, nullptr, mirror));
    HR(CreateShaderResourceView(md3dDevice, mirror.GetImages(), mirror.GetImageCount(), mirror.GetMetadata(), &mMirrorDiffuseMapSRV));
}

void Mirror::BuildRoomGeometryBuffers()
{
	// Create and specify geometry.  For this sample we draw a floor
	// and a wall with a mirror on it.  We put the floor, wall, and
	// mirror geometry in one vertex buffer.
	//
	//   |--------------|
	//   |              |
    //   |----|----|----|
    //   |Wall|Mirr|Wall|
	//   |    | or |    |
    //   /--------------/
    //  /   Floor      /
	// /--------------/

    Vertex::Basic32 v[30];

    // Floor: Observe we tile texture coordinates.
    v[0] = Vertex::Basic32(-3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f);
    v[1] = Vertex::Basic32(-3.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
    v[2] = Vertex::Basic32(7.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f);

    v[3] = Vertex::Basic32(-3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f);
    v[4] = Vertex::Basic32(7.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f);
    v[5] = Vertex::Basic32(7.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 4.0f, 4.0f);

    // Wall: Observe we tile texture coordinates, and that we
    // leave a gap in the middle for the mirror.
    v[6]  = Vertex::Basic32(-3.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
    v[7]  = Vertex::Basic32(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
    v[8]  = Vertex::Basic32(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 0.0f);

    v[9]  = Vertex::Basic32(-3.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
    v[10] = Vertex::Basic32(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 0.0f);
    v[11] = Vertex::Basic32(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 2.0f);

    v[12] = Vertex::Basic32(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
    v[13] = Vertex::Basic32(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
    v[14] = Vertex::Basic32(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f);

    v[15] = Vertex::Basic32(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
    v[16] = Vertex::Basic32(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f);
    v[17] = Vertex::Basic32(7.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 2.0f);

    v[18] = Vertex::Basic32(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
    v[19] = Vertex::Basic32(-3.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
    v[20] = Vertex::Basic32(7.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 0.0f);

    v[21] = Vertex::Basic32(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
    v[22] = Vertex::Basic32(7.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 0.0f);
    v[23] = Vertex::Basic32(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 1.0f);

    // Mirror
    v[24] = Vertex::Basic32(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
    v[25] = Vertex::Basic32(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
    v[26] = Vertex::Basic32(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);

    v[27] = Vertex::Basic32(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
    v[28] = Vertex::Basic32(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
    v[29] = Vertex::Basic32(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex::Basic32) * 30;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = v;

    HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mRoomVertexBuffer));
}

void Mirror::BuildSkullGeometryBuffers()
{
    std::ifstream fin("models/skull.txt");

    if (!fin)
    {
        MessageBox(0, L"models/skull.txt not found.", 0, 0);
        return;
    }

    UINT vCount = 0;
    UINT tCount = 0;
    std::string ignore;

    fin >> ignore >> vCount;
    fin >> ignore >> tCount;
    fin >> ignore >> ignore >> ignore >> ignore;

    std::vector<Vertex::Basic32> vertices(vCount);
    for (UINT i = 0; i < vCount; ++i)
    {
        fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
        fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
    }

    fin >> ignore >> ignore >> ignore;

    mSkullIndexCount = tCount * 3;
    std::vector<UINT> indices(mSkullIndexCount);
    for (UINT i = 0; i < tCount; ++i)
    {
        fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
    }

    fin.close();

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex::Basic32) * vCount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA vInitData;
    vInitData.pSysMem = &vertices[0];

    HR(md3dDevice->CreateBuffer(&vbd, &vInitData, &mSkullVertexBuffer));

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * mSkullIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA iInitData;
    iInitData.pSysMem = &indices[0];

    HR(md3dDevice->CreateBuffer(&ibd, &iInitData, &mSkullIndexBuffer));
}