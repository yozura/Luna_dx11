#include "DynamicCubeMap.h"

using namespace DirectX;

DynamicCubeMap::DynamicCubeMap(HINSTANCE hInstance)
    : D3DApp(hInstance)
    , mShapesVB(0), mShapesIB(0)
    , mSkullVB(0), mSkullIB(0)
    , mSkullIndexCount(0), mFloorTexSRV(0), mStoneTexSRV(0)
    , mDynamicCubeMapDSV(0), mDynamicCubeMapSRV(0)
    , mBrickTexSRV(0), mLightCount(3)
{
    mMainWndCaption = L"DynamicCubeMap";

    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    mCam.SetPosition(0.0f, 2.0f, -15.0f);

    BuildCubeFaceFrame(0.0f, 2.0f, 0.0f);

    for (int i = 0; i < 6; ++i)
    {
        mDynamicCubeMapRTV[i] = 0;
    }

    XMMATRIX I = XMMatrixIdentity();
    XMStoreFloat4x4(&mGridWorld, I);

    XMMATRIX boxScale  = XMMatrixScaling(3.0f, 1.0f, 3.0f);
    XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
    XMStoreFloat4x4(&mBoxWorld, XMMatrixMultiply(boxScale, boxOffset));

    XMMATRIX centerSphereScale  = XMMatrixScaling(2.0f, 2.0f, 2.0f);
    XMMATRIX centerSphereOffset = XMMatrixTranslation(0.0f, 2.0f, 0.0f);
    XMStoreFloat4x4(&mCenterSphereWorld, XMMatrixMultiply(centerSphereScale, centerSphereOffset));

    for (int i = 0; i < 5; ++i)
    {
        XMStoreFloat4x4(&mCylinderWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f));
        XMStoreFloat4x4(&mCylinderWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f));

        XMStoreFloat4x4(&mSphereWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f));
        XMStoreFloat4x4(&mSphereWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f));
    }

    mDirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirLights[0].Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    mDirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    mDirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

    mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mDirLights[1].Diffuse = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
    mDirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
    mDirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

    mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mDirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mDirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

    mGridMat.Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    mGridMat.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    mGridMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
    mGridMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

    mCylinderMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mCylinderMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mCylinderMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
    mCylinderMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

    mSphereMat.Ambient = XMFLOAT4(0.6f, 0.8f, 1.0f, 1.0f);
    mSphereMat.Diffuse = XMFLOAT4(0.6f, 0.8f, 1.0f, 1.0f);
    mSphereMat.Specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f);
    mSphereMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

    mBoxMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mBoxMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mBoxMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
    mBoxMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

    mSkullMat.Ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
    mSkullMat.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    mSkullMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
    mSkullMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

    mCenterSphereMat.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mCenterSphereMat.Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mCenterSphereMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
    mCenterSphereMat.Reflect = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
}

DynamicCubeMap::~DynamicCubeMap()
{
    SafeDelete(mSky);

    ReleaseCOM(mShapesVB);
    ReleaseCOM(mShapesIB);
    ReleaseCOM(mSkullVB);
    ReleaseCOM(mSkullIB);
    ReleaseCOM(mFloorTexSRV);
    ReleaseCOM(mStoneTexSRV);
    ReleaseCOM(mBrickTexSRV);

    ReleaseCOM(mDynamicCubeMapDSV);
    ReleaseCOM(mDynamicCubeMapSRV);

    for (int i = 0; i < 6; ++i)
        ReleaseCOM(mDynamicCubeMapRTV[i]);

    Effects::DestroyAll();
    InputLayouts::DestroyAll();
}

bool DynamicCubeMap::Init()
{
    if (!D3DApp::Init())
        return false;

    Effects::InitAll(md3dDevice);
    InputLayouts::InitAll(md3dDevice);

    mSky = new Sky(md3dDevice, L"textures/sunsetcube1024.dds", 5000.0f);

    ScratchImage floor;
    HR(LoadFromDDSFile(L"textures/floor.dds", DDS_FLAGS_NONE, nullptr, floor));
    HR(CreateShaderResourceView(md3dDevice, floor.GetImages(), floor.GetImageCount(), floor.GetMetadata(), &mFloorTexSRV));

    ScratchImage stone;
    HR(LoadFromDDSFile(L"textures/stone.dds", DDS_FLAGS_NONE, nullptr, stone));
    HR(CreateShaderResourceView(md3dDevice, stone.GetImages(), stone.GetImageCount(), stone.GetMetadata(), &mStoneTexSRV));

    ScratchImage bricks;
    HR(LoadFromDDSFile(L"textures/bricks.dds", DDS_FLAGS_NONE, nullptr, bricks));
    HR(CreateShaderResourceView(md3dDevice, bricks.GetImages(), bricks.GetImageCount(), bricks.GetMetadata(), &mBrickTexSRV));

    BuildDynamicCubeMapView();

    BuildShapeGeometryBuffers();
    BuildSkullGeometryBuffers();

    return true;
}

void DynamicCubeMap::OnResize()
{
    D3DApp::OnResize();

    mCam.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
}

void DynamicCubeMap::UpdateScene(float dt)
{
    if (GetAsyncKeyState('W') & 0x8000)
        mCam.Walk(10.0f * dt);
    if (GetAsyncKeyState('S') & 0x8000)
        mCam.Walk(-10.0f * dt);
    if (GetAsyncKeyState('A') & 0x8000)
        mCam.Strafe(-10.0f * dt);
    if (GetAsyncKeyState('D') & 0x8000)
        mCam.Strafe(10.0f * dt);

    if (GetAsyncKeyState('0') & 0x8000)
        mLightCount = 0;
    if (GetAsyncKeyState('1') & 0x8000)
        mLightCount = 1;
    if (GetAsyncKeyState('2') & 0x8000)
        mLightCount = 2;
    if (GetAsyncKeyState('3') & 0x8000)
        mLightCount = 3;

    XMMATRIX skullScale = XMMatrixScaling(0.2f, 0.2f, 0.2f);
    XMMATRIX skullOffset = XMMatrixTranslation(3.0f, 2.0f, 0.0f);
    XMMATRIX skullLocalRotate = XMMatrixRotationY(2.0f * mTimer.TotalTime());
    XMMATRIX skullGlobalRotate = XMMatrixRotationY(0.5f * mTimer.TotalTime());
    XMStoreFloat4x4(&mSkullWorld, skullScale * skullLocalRotate * skullOffset * skullGlobalRotate);

    mCam.UpdateViewMatrix();
}

void DynamicCubeMap::DrawScene()
{
    ID3D11RenderTargetView* renderTargets[1];

    // Generate the Cube map
    md3dImmediateContext->RSSetViewports(1, &mCubeMapViewport);
    for (int i = 0; i < 6; ++i)
    {
        // Clear cube map face and depth buffer
        md3dImmediateContext->ClearRenderTargetView(mDynamicCubeMapRTV[i], reinterpret_cast<const float*>(&Colors::Silver));
        md3dImmediateContext->ClearDepthStencilView(mDynamicCubeMapDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        // Bind cube map face as render target.
        renderTargets[0] = mDynamicCubeMapRTV[i];
        md3dImmediateContext->OMSetRenderTargets(1, renderTargets, mDynamicCubeMapDSV);

        // Draw the scene with the exception of the center sphere to this cube map face.
        DrawScene(mCubeMapCamera[i], false);
    }

    // Restore old viewport and render targets.
    md3dImmediateContext->RSSetViewports(1, &mScreenViewport);
    renderTargets[0] = mRenderTargetView;
    md3dImmediateContext->OMSetRenderTargets(1, renderTargets, mDepthStencilView);

    // Have hardware generate lower mipmap levels of cube map.
    md3dImmediateContext->GenerateMips(mDynamicCubeMapSRV);

    // Now draw the scene as normal, but with the center sphere.
    md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
    md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    DrawScene(mCam, true);

    HR(mSwapChain->Present(0, 0));
}

void DynamicCubeMap::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void DynamicCubeMap::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void DynamicCubeMap::OnMouseMove(WPARAM btnState, int x, int y)
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

void DynamicCubeMap::DrawScene(const Camera& camera, bool drawCenterSphere)
{
    md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    UINT stride = sizeof(Vertex::Basic32);
    UINT offset = 0;

    XMMATRIX view = camera.View();
    XMMATRIX proj = camera.Proj();
    XMMATRIX viewProj = camera.ViewProj();

    float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

    // Set per frame constants.
    Effects::BasicFX->SetDirLights(mDirLights);
    Effects::BasicFX->SetEyePosW(mCam.GetPosition());

    ID3DX11EffectTechnique* activeTexTech = Effects::BasicFX->Light1TexTech;
    ID3DX11EffectTechnique* activeSkullTech = Effects::BasicFX->Light1Tech;
    ID3DX11EffectTechnique* activeReflectTech = Effects::BasicFX->Light1ReflectTech;
    switch (mLightCount)
    {
    case 1:
        activeTexTech = Effects::BasicFX->Light1TexTech;
        activeSkullTech = Effects::BasicFX->Light1Tech;
        activeReflectTech = Effects::BasicFX->Light1ReflectTech;
        break;
    case 2:
        activeTexTech = Effects::BasicFX->Light2TexTech;
        activeSkullTech = Effects::BasicFX->Light2Tech;
        activeReflectTech = Effects::BasicFX->Light2ReflectTech;
        break;
    case 3:
        activeTexTech = Effects::BasicFX->Light3TexTech;
        activeSkullTech = Effects::BasicFX->Light3Tech;
        activeReflectTech = Effects::BasicFX->Light3ReflectTech;
        break;
    }

    XMMATRIX world;
    XMMATRIX worldInvTranspose;
    XMMATRIX worldViewProj;

    // Draw the skull.
    D3DX11_TECHNIQUE_DESC techDesc;
    activeSkullTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        md3dImmediateContext->IASetVertexBuffers(0, 1, &mSkullVB, &stride, &offset);
        md3dImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);

        world = XMLoadFloat4x4(&mSkullWorld);
        worldInvTranspose = MathHelper::InverseTranspose(world);
        worldViewProj = world * view * proj;

        Effects::BasicFX->SetWorld(world);
        Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::BasicFX->SetWorldViewProj(worldViewProj);
        Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
        Effects::BasicFX->SetMaterial(mSkullMat);

        activeSkullTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);
    }

    md3dImmediateContext->IASetVertexBuffers(0, 1, &mShapesVB, &stride, &offset);
    md3dImmediateContext->IASetIndexBuffer(mShapesIB, DXGI_FORMAT_R32_UINT, 0);

    //
    // Draw the grid, cylinders, spheres and box without any cubemap reflection.
    // 

    activeTexTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        // Draw the grid.
        world = XMLoadFloat4x4(&mGridWorld);
        worldInvTranspose = MathHelper::InverseTranspose(world);
        worldViewProj = world * view * proj;

        Effects::BasicFX->SetWorld(world);
        Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::BasicFX->SetWorldViewProj(worldViewProj);
        Effects::BasicFX->SetTexTransform(XMMatrixScaling(6.0f, 8.0f, 1.0f));
        Effects::BasicFX->SetMaterial(mGridMat);
        Effects::BasicFX->SetDiffuseMap(mFloorTexSRV);

        activeTexTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

        // Draw the box.
        world = XMLoadFloat4x4(&mBoxWorld);
        worldInvTranspose = MathHelper::InverseTranspose(world);
        worldViewProj = world * view * proj;

        Effects::BasicFX->SetWorld(world);
        Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::BasicFX->SetWorldViewProj(worldViewProj);
        Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
        Effects::BasicFX->SetMaterial(mBoxMat);
        Effects::BasicFX->SetDiffuseMap(mStoneTexSRV);

        activeTexTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

        // Draw the cylinders.
        for (int i = 0; i < 10; ++i)
        {
            world = XMLoadFloat4x4(&mCylinderWorld[i]);
            worldInvTranspose = MathHelper::InverseTranspose(world);
            worldViewProj = world * view * proj;

            Effects::BasicFX->SetWorld(world);
            Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
            Effects::BasicFX->SetWorldViewProj(worldViewProj);
            Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
            Effects::BasicFX->SetMaterial(mCylinderMat);
            Effects::BasicFX->SetDiffuseMap(mBrickTexSRV);

            activeTexTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
            md3dImmediateContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
        }

        // Draw the spheres.
        for (int i = 0; i < 10; ++i)
        {
            world = XMLoadFloat4x4(&mSphereWorld[i]);
            worldInvTranspose = MathHelper::InverseTranspose(world);
            worldViewProj = world * view * proj;

            Effects::BasicFX->SetWorld(world);
            Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
            Effects::BasicFX->SetWorldViewProj(worldViewProj);
            Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
            Effects::BasicFX->SetMaterial(mSphereMat);
            Effects::BasicFX->SetDiffuseMap(mStoneTexSRV);

            activeTexTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
            md3dImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
        }
    }

    // Draw the center sphere with the dynamic cube map.
    if (drawCenterSphere)
    {
        activeReflectTech->GetDesc(&techDesc);
        for (UINT p = 0; p < techDesc.Passes; ++p)
        {
            // Draw the center sphere.
            world = XMLoadFloat4x4(&mCenterSphereWorld);
            worldInvTranspose = MathHelper::InverseTranspose(world);
            worldViewProj = world * view * proj;

            Effects::BasicFX->SetWorld(world);
            Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
            Effects::BasicFX->SetWorldViewProj(worldViewProj);
            Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
            Effects::BasicFX->SetMaterial(mCenterSphereMat);
            Effects::BasicFX->SetDiffuseMap(mStoneTexSRV);
            Effects::BasicFX->SetCubeMap(mDynamicCubeMapSRV);

            activeReflectTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
            md3dImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
        }
    }

    mSky->Draw(md3dImmediateContext, camera);

    // restore default states, as the SkyFX changes them in the effect file.
    md3dImmediateContext->RSSetState(0);
    md3dImmediateContext->OMSetDepthStencilState(0, 0);
}

void DynamicCubeMap::BuildCubeFaceFrame(float x, float y, float z)
{
    // Generate the cube map about the given position.
    XMFLOAT3 center(x, y, z);
    XMFLOAT3 worldUp(0.0f, 1.0f, 0.0f);

    // Look along each coordinate axis.
    XMFLOAT3 targets[6] =
    {
        XMFLOAT3(x + 1.0f, y, z), // +X
        XMFLOAT3(x - 1.0f, y, z), // -X
        XMFLOAT3(x, y + 1.0f, z), // +Y
        XMFLOAT3(x, y - 1.0f, z), // -Y
        XMFLOAT3(x, y, z + 1.0f), // +Z
        XMFLOAT3(x, y, z - 1.0f) // -Z
    };

    // Use world up vector (0, 1, 0) for all direction except +Y/-Y.
    // In these cases, we are looking down +Y or -Y, so we need a diffrent "up" vector.
    XMFLOAT3 ups[6] =
    {
        XMFLOAT3(0.0f, 1.0f, 0.0f),
        XMFLOAT3(0.0f, 1.0f, 0.0f),
        XMFLOAT3(0.0f, 0.0f, -1.0f),
        XMFLOAT3(0.0f, 0.0f, 1.0f),
        XMFLOAT3(0.0f, 1.0f, 0.0f),
        XMFLOAT3(0.0f, 1.0f, 0.0f)
    };

    for (int i = 0; i < 6; ++i)
    {
        mCubeMapCamera[i].LookAt(center, targets[i], ups[i]);
        mCubeMapCamera[i].SetLens(0.5f * XM_PI, 1.0f, 1.0f, 1000.0f);
        mCubeMapCamera[i].UpdateViewMatrix();
    }
}

void DynamicCubeMap::BuildDynamicCubeMapView()
{
    // Cube map is a special texture array with 6 elem.

    D3D11_TEXTURE2D_DESC texDesc;
    texDesc.Width = CubeMapSize;
    texDesc.Height = CubeMapSize;
    texDesc.MipLevels = 0;
    texDesc.ArraySize = 6;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE;

    ID3D11Texture2D* cubeTex = 0;
    HR(md3dDevice->CreateTexture2D(&texDesc, 0, &cubeTex));

    // Create render target view to each cube map face.
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    rtvDesc.Format = texDesc.Format;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
    rtvDesc.Texture2DArray.ArraySize = 1;
    rtvDesc.Texture2DArray.MipSlice = 0;

    for (int i = 0; i < 6; ++i)
    {
        rtvDesc.Texture2DArray.FirstArraySlice = i;
        HR(md3dDevice->CreateRenderTargetView(cubeTex, &rtvDesc, &mDynamicCubeMapRTV[i]));
    }

    // Create a shader resource view to cube map.
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MostDetailedMip = 0;
    srvDesc.TextureCube.MipLevels = -1;
    HR(md3dDevice->CreateShaderResourceView(cubeTex, &srvDesc, &mDynamicCubeMapSRV));
    
    ReleaseCOM(cubeTex);

    // We need a depth texture for rendering the scene into the cubemap
    // that has the same resolution as the cubemap faces.
    D3D11_TEXTURE2D_DESC depthTexDesc;
    depthTexDesc.Width = CubeMapSize;
    depthTexDesc.Height = CubeMapSize;
    depthTexDesc.MipLevels = 1;
    depthTexDesc.ArraySize = 1;
    depthTexDesc.SampleDesc.Count = 1;
    depthTexDesc.SampleDesc.Quality = 0;
    depthTexDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
    depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthTexDesc.CPUAccessFlags = 0;
    depthTexDesc.MiscFlags = 0;

    ID3D11Texture2D* depthTex = 0;
    HR(md3dDevice->CreateTexture2D(&depthTexDesc, 0, &depthTex));

    // Create the depth stencil view for the entire cube
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Format = depthTexDesc.Format;
    dsvDesc.Flags = 0;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
    HR(md3dDevice->CreateDepthStencilView(depthTex, &dsvDesc, &mDynamicCubeMapDSV));
    ReleaseCOM(depthTex);


    // Viewport for drawing into cubemap.
    mCubeMapViewport.TopLeftX = 0.0f;
    mCubeMapViewport.TopLeftY = 0.0f;
    mCubeMapViewport.Width    = (float)CubeMapSize;
    mCubeMapViewport.Height   = (float)CubeMapSize;
    mCubeMapViewport.MinDepth = 0.0f;
    mCubeMapViewport.MaxDepth = 1.0f;
}

void DynamicCubeMap::BuildShapeGeometryBuffers()
{
    GeometryGenerator::MeshData box;
    GeometryGenerator::MeshData grid;
    GeometryGenerator::MeshData sphere;
    GeometryGenerator::MeshData cylinder;

    GeometryGenerator geoGen;
    geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
    geoGen.CreateGrid(20.0f, 30.0f, 60, 40, grid);
    geoGen.CreateSphere(0.5f, 20, 20, sphere);
    geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20, cylinder);

    mBoxVertexOffset      = 0;
    mGridVertexOffset     = box.Vertices.size();
    mSphereVertexOffset   = mGridVertexOffset + grid.Vertices.size();
    mCylinderVertexOffset = mSphereVertexOffset + sphere.Vertices.size();

    mBoxIndexCount      = box.Indices.size();
    mGridIndexCount     = grid.Indices.size();
    mSphereIndexCount   = sphere.Indices.size();
    mCylinderIndexCount = cylinder.Indices.size();

    mBoxIndexOffset      = 0;
    mGridIndexOffset     = mBoxIndexOffset;
    mSphereIndexOffset   = mGridIndexOffset + mGridIndexCount;
    mCylinderIndexOffset = mSphereIndexOffset + mSphereIndexCount;

    UINT totalVertexCount = box.Vertices.size()
                          + grid.Vertices.size()
                          + sphere.Vertices.size()
                          + cylinder.Vertices.size();

    UINT totalIndexCount = mBoxIndexCount 
                         + mGridIndexCount 
                         + mSphereIndexCount 
                         + mCylinderIndexCount;

    std::vector<Vertex::Basic32> vertices(totalVertexCount);
    UINT k = 0;
    for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos    = box.Vertices[i].Position;
        vertices[k].Normal = box.Vertices[i].Normal;
        vertices[k].TexC   = box.Vertices[i].TexC;
    }

    for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos    = grid.Vertices[i].Position;
        vertices[k].Normal = grid.Vertices[i].Normal;
        vertices[k].TexC   = grid.Vertices[i].TexC;
    }
    
    for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos    = sphere.Vertices[i].Position;
        vertices[k].Normal = sphere.Vertices[i].Normal;
        vertices[k].TexC   = sphere.Vertices[i].TexC;
    }

    for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos    = cylinder.Vertices[i].Position;
        vertices[k].Normal = cylinder.Vertices[i].Normal;
        vertices[k].TexC   = cylinder.Vertices[i].TexC;
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex::Basic32) * totalVertexCount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA vInitData;
    vInitData.pSysMem = &vertices[0];
    HR(md3dDevice->CreateBuffer(&vbd, &vInitData, &mShapesVB));

    std::vector<UINT> indices;
    indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());
    indices.insert(indices.end(), grid.Indices.begin(), grid.Indices.end());
    indices.insert(indices.end(), sphere.Indices.begin(), sphere.Indices.end());
    indices.insert(indices.end(), cylinder.Indices.begin(), cylinder.Indices.end());

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA iInitData;
    iInitData.pSysMem = &indices[0];
    HR(md3dDevice->CreateBuffer(&ibd, &iInitData, &mShapesIB));
}

void DynamicCubeMap::BuildSkullGeometryBuffers()
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

    HR(md3dDevice->CreateBuffer(&vbd, &vInitData, &mSkullVB));

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * mSkullIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA iInitData;
    iInitData.pSysMem = &indices[0];

    HR(md3dDevice->CreateBuffer(&ibd, &iInitData, &mSkullIB));
}