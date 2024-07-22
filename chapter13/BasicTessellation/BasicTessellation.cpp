#include "BasicTessellation.h"

using namespace DirectX;

BasicTessellation::BasicTessellation(HINSTANCE hInstance)
    : D3DApp(hInstance)
    , mLandVB(0), mLandIB(0), mWavesVB(0), mWavesIB(0)
    , mBoxVB(0), mBoxIB(0), mScreenQuadVB(0), mScreenQuadIB(0)
    , mGrassMapSRV(0), mWaterMapSRV(0), mCrateSRV(0)
    , mOffscreenSRV(0), mOffscreenUAV(0), mOffscreenRTV(0)
    , mWaterTexOffset(0.0f, 0.0f), mEyePosW(0.0f, 0.0f, 0.0f)
    , mLandIndexCount(0), mWavesIndexCount(0), mRenderOptions(RenderOptions::TexturesAndFog)
    , mTheta(1.3f * MathHelper::Pi), mPhi(0.4f * MathHelper::Pi), mRadius(80.0f)

{
    mMainWndCaption = L"BasicTessellation";
    mEnable4xMsaa   = false;

    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    XMMATRIX I = XMMatrixIdentity();
    XMStoreFloat4x4(&mLandWorld, I);
    XMStoreFloat4x4(&mWavesWorld, I);
    XMStoreFloat4x4(&mView, I);
    XMStoreFloat4x4(&mProj, I);

    XMMATRIX boxScale = XMMatrixScaling(15.0f, 15.0f, 15.0f);
    XMMATRIX boxOffset = XMMatrixTranslation(8.0f, 5.0f, -15.0f);
    XMStoreFloat4x4(&mBoxWorld, boxScale * boxOffset);

    XMMATRIX grassTexScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);
    XMStoreFloat4x4(&mGrassTexTransform, grassTexScale);

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

    mLandMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    mLandMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mLandMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

    mWavesMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    mWavesMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
    mWavesMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f);

    mBoxMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    mBoxMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mBoxMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
}

BasicTessellation::~BasicTessellation()
{
    md3dImmediateContext->ClearState();
    ReleaseCOM(mLandVB);
    ReleaseCOM(mLandIB);
    ReleaseCOM(mWavesVB);
    ReleaseCOM(mWavesIB);
    ReleaseCOM(mBoxVB);
    ReleaseCOM(mBoxIB);
    ReleaseCOM(mScreenQuadVB);
    ReleaseCOM(mScreenQuadIB);

    ReleaseCOM(mGrassMapSRV);
    ReleaseCOM(mWaterMapSRV);
    ReleaseCOM(mCrateSRV);
    
    ReleaseCOM(mOffscreenSRV);
    ReleaseCOM(mOffscreenUAV);
    ReleaseCOM(mOffscreenRTV);

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

    ScratchImage grass;
    HR(LoadFromDDSFile(L"textures/grass.dds", DDS_FLAGS_NONE, nullptr, grass));
    HR(CreateShaderResourceView(md3dDevice, grass.GetImages(), grass.GetImageCount(), grass.GetMetadata(), &mGrassMapSRV));

    ScratchImage water;
    HR(LoadFromDDSFile(L"textures/water2.dds", DDS_FLAGS_NONE, nullptr, water));
    HR(CreateShaderResourceView(md3dDevice, water.GetImages(), water.GetImageCount(), water.GetMetadata(), &mWaterMapSRV));

    ScratchImage fence;
    HR(LoadFromDDSFile(L"textures/WireFence.dds", DDS_FLAGS_NONE, nullptr, fence));
    HR(CreateShaderResourceView(md3dDevice, fence.GetImages(), fence.GetImageCount(), fence.GetMetadata(), &mCrateSRV));

    mWaves.Init(160, 160, 1.0f, 0.03f, 3.25f, 0.4f);

    BuildLandGeometryBuffers();
    BuildWaveGeometryBuffers();
    BuildCrateGeometryBuffers();
    BuildScreenQuadGeometryBuffers();
    BuildOffscreenViews();

    return true;
}

void BasicTessellation::OnResize()
{
    D3DApp::OnResize();

    // Recreate the resources that depend on the client area size.
    BuildOffscreenViews();
    mBlur.Init(md3dDevice, mClientWidth, mClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM);

    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj, P);
}

void BasicTessellation::UpdateScene(float dt)
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

    static float t_base = 0.0f;
    if ((mTimer.TotalTime() - t_base) >= 0.1f)
    {
        t_base += 0.1f;

        DWORD i = 5 + rand() % (mWaves.RowCount() - 10);
        DWORD j = 5 + rand() % (mWaves.ColumnCount() - 10);
        float r = MathHelper::RandF(0.5f, 1.0f);

        mWaves.Disturb(i, j, r);
    }

    mWaves.Update(dt);

    D3D11_MAPPED_SUBRESOURCE mappedData;
    md3dImmediateContext->Map(mWavesVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);

    Vertex::Basic32* v = reinterpret_cast<Vertex::Basic32*>(mappedData.pData);
    for (UINT i = 0; i < mWaves.VertexCount(); ++i)
    {
        v[i].Pos = mWaves[i];
        v[i].Normal = mWaves.Normal(i);

        v[i].Tex.x = 0.5f + mWaves[i].x / mWaves.Width();
        v[i].Tex.y = 0.5f - mWaves[i].z / mWaves.Depth();
    }

    md3dImmediateContext->Unmap(mWavesVB, 0);

    XMMATRIX wavesScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);

    mWaterTexOffset.y += 0.05f * dt;
    mWaterTexOffset.x += 0.1f * dt;

    XMMATRIX wavesOffset = XMMatrixTranslation(mWaterTexOffset.x, mWaterTexOffset.y, 0.0f);

    XMStoreFloat4x4(&mWaterTexTransform, wavesScale * wavesOffset);

    if (GetAsyncKeyState('1') & 0x8000)
        mRenderOptions = RenderOptions::Lighting;
    if (GetAsyncKeyState('2') & 0x8000)
        mRenderOptions = RenderOptions::Textures;
    if (GetAsyncKeyState('3') & 0x8000)
        mRenderOptions = RenderOptions::TexturesAndFog;
}

void BasicTessellation::DrawScene()
{
    ID3D11RenderTargetView* renderTargets[1] = { mOffscreenRTV };
    md3dImmediateContext->OMSetRenderTargets(1, renderTargets, mDepthStencilView);

    md3dImmediateContext->ClearRenderTargetView(mOffscreenRTV, reinterpret_cast<const float*>(&Colors::Silver));
    md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    DrawWrapper();

    renderTargets[0] = mRenderTargetView;
    md3dImmediateContext->OMSetRenderTargets(1, renderTargets, mDepthStencilView);
    
    // mBlur.SetGaussianWeights(4.0f);
    mBlur.BlurInPlace(md3dImmediateContext, mOffscreenSRV, mOffscreenUAV, 4);
    
    md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
    md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    DrawScreenQuad();

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
        // Make each pixel correspond to 0.01 unit in the scene.
        float dx = 0.1f * static_cast<float>(x - mLastMousePos.x);
        float dy = 0.1f * static_cast<float>(y - mLastMousePos.y);

        // Update the camera radius based on input.
        mRadius += dx - dy;

        // Restrict the radius.
        mRadius = MathHelper::Clamp(mRadius, 20.0f, 500.0f);
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}

void BasicTessellation::DrawWrapper()
{
    md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

    UINT stride = sizeof(Vertex::Basic32);
    UINT offset = 0;
    
    XMMATRIX view = XMLoadFloat4x4(&mView);
    XMMATRIX proj = XMLoadFloat4x4(&mProj);

    Effects::BasicFX->SetDirLights(mDirLights);
    Effects::BasicFX->SetEyePosW(mEyePosW);
    Effects::BasicFX->SetFogColor(Colors::Silver);
    Effects::BasicFX->SetFogStart(30.0f);
    Effects::BasicFX->SetFogRange(175.0f);

    ID3DX11EffectTechnique* boxTech = 0;
    ID3DX11EffectTechnique* landAndWavesTech = 0;

    switch (mRenderOptions)
    {
    case RenderOptions::Lighting:
        boxTech = Effects::BasicFX->Light3Tech;
        landAndWavesTech = Effects::BasicFX->Light3Tech;
        break;
    case RenderOptions::Textures:
        boxTech = Effects::BasicFX->Light3TexAlphaClipTech;
        landAndWavesTech = Effects::BasicFX->Light3TexTech;
        break;
    case RenderOptions::TexturesAndFog:
        boxTech = Effects::BasicFX->Light3TexAlphaClipFogTech;
        landAndWavesTech = Effects::BasicFX->Light3TexFogTech;
        break;
    }

    D3DX11_TECHNIQUE_DESC techDesc;

    // Draw the box with alpha clipping
    boxTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
        md3dImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);

        XMMATRIX world = XMLoadFloat4x4(&mBoxWorld);
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
        XMMATRIX worldViewProj = world * view * proj;

        Effects::BasicFX->SetWorld(world);
        Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::BasicFX->SetWorldViewProj(worldViewProj);
        Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
        Effects::BasicFX->SetMaterial(mBoxMat);
        Effects::BasicFX->SetDiffuseMap(mCrateSRV);

        md3dImmediateContext->RSSetState(RenderStates::NoCullRS);
        boxTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexed(36, 0, 0);

        // Restore default Rasterizer State
        md3dImmediateContext->RSSetState(0);
    }

    // Draw the hills and waves
    landAndWavesTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        md3dImmediateContext->IASetVertexBuffers(0, 1, &mLandVB, &stride, &offset);
        md3dImmediateContext->IASetIndexBuffer(mLandIB, DXGI_FORMAT_R32_UINT, 0);

        // Land
        XMMATRIX world = XMLoadFloat4x4(&mLandWorld);
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
        XMMATRIX worldViewProj = world * view * proj;

        Effects::BasicFX->SetWorld(world);
        Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::BasicFX->SetWorldViewProj(worldViewProj);
        Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&mGrassTexTransform));
        Effects::BasicFX->SetMaterial(mLandMat);
        Effects::BasicFX->SetDiffuseMap(mGrassMapSRV);

        landAndWavesTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexed(mLandIndexCount, 0, 0);

        // Waves
        md3dImmediateContext->IASetVertexBuffers(0, 1, &mWavesVB, &stride, &offset);
        md3dImmediateContext->IASetIndexBuffer(mWavesIB, DXGI_FORMAT_R32_UINT, 0);

        world = XMLoadFloat4x4(&mWavesWorld);
        worldInvTranspose = MathHelper::InverseTranspose(world);
        worldViewProj = world * view * proj;

        Effects::BasicFX->SetWorld(world);
        Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::BasicFX->SetWorldViewProj(worldViewProj);
        Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&mWaterTexTransform));
        Effects::BasicFX->SetMaterial(mWavesMat);
        Effects::BasicFX->SetDiffuseMap(mWaterMapSRV);

        md3dImmediateContext->OMSetBlendState(RenderStates::TransparentBS, blendFactor, 0xffffffff);
        landAndWavesTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexed(3 * mWaves.TriangleCount(), 0, 0);

        // Restore default Blend State
        md3dImmediateContext->OMSetBlendState(0, blendFactor, 0xffffffff);
    }
}

void BasicTessellation::DrawScreenQuad()
{
    md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    UINT stride = sizeof(Vertex::Basic32);
    UINT offset = 0;

    XMMATRIX identity = XMMatrixIdentity();

    ID3DX11EffectTechnique* texOnlyTech = Effects::BasicFX->Light0TexTech;
    D3DX11_TECHNIQUE_DESC techDesc;

    texOnlyTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        md3dImmediateContext->IASetVertexBuffers(0, 1, &mScreenQuadVB, &stride, &offset);
        md3dImmediateContext->IASetIndexBuffer(mScreenQuadIB, DXGI_FORMAT_R32_UINT, 0);

        Effects::BasicFX->SetWorld(identity);
        Effects::BasicFX->SetWorldInvTranspose(identity);
        Effects::BasicFX->SetWorldViewProj(identity);
        Effects::BasicFX->SetTexTransform(identity);
        Effects::BasicFX->SetDiffuseMap(mBlur.GetBlurredOutput());

        texOnlyTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexed(6, 0, 0);
    }
}

float BasicTessellation::GetHillHeight(float x, float z) const
{
    return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}

XMFLOAT3 BasicTessellation::GetHillNormal(float x, float z) const
{
    // n = (-df/dx, 1, -df/dz)
    XMFLOAT3 n(
        -0.03f * z * cosf(0.1f * x) - 0.3f * cosf(0.1f * z),
        1.0f,
        -0.3f * sinf(0.1f * x) + 0.03f * x * sinf(0.1f * z));

    XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
    XMStoreFloat3(&n, unitNormal);

    return n;
}

void BasicTessellation::BuildLandGeometryBuffers()
{
    GeometryGenerator::MeshData grid;
    GeometryGenerator geoGen;

    geoGen.CreateGrid(160.0f, 160.0f, 50, 50, grid);

    mLandIndexCount = grid.Indices.size();

    std::vector<Vertex::Basic32> vertices(grid.Vertices.size());
    for (size_t i = 0; i < grid.Vertices.size(); ++i)
    {
        XMFLOAT3 p = grid.Vertices[i].Position;
        p.y = GetHillHeight(p.x, p.z);

        vertices[i].Pos = p;
        vertices[i].Normal = GetHillNormal(p.x, p.z);
        vertices[i].Tex = grid.Vertices[i].TexC;
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex::Basic32) * grid.Vertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA vInitData;
    vInitData.pSysMem = &vertices[0];

    HR(md3dDevice->CreateBuffer(&vbd, &vInitData, &mLandVB));

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * mLandIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA iInitData;
    iInitData.pSysMem = &grid.Indices[0];

    HR(md3dDevice->CreateBuffer(&ibd, &iInitData, &mLandIB));
}

void BasicTessellation::BuildWaveGeometryBuffers()
{
    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_DYNAMIC;
    vbd.ByteWidth = sizeof(Vertex::Basic32) * mWaves.VertexCount();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vbd.MiscFlags = 0;

    HR(md3dDevice->CreateBuffer(&vbd, 0, &mWavesVB));

    std::vector<UINT> indices(3 * mWaves.TriangleCount());
    UINT m = mWaves.RowCount();
    UINT n = mWaves.ColumnCount();
    int k = 0;
    for (UINT i = 0; i < m - 1; ++i)
    {
        for (DWORD j = 0; j < n - 1; ++j)
        {
            indices[k] = i * n + j;
            indices[k + 1] = i * n + j + 1;
            indices[k + 2] = (i + 1) * n + j;

            indices[k + 3] = (i + 1) * n + j;
            indices[k + 4] = i * n + j + 1;
            indices[k + 5] = (i + 1) * n + j + 1;

            k += 6; // next quad
        }
    }

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * indices.size();
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &indices[0];
    HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mWavesIB));
}

void BasicTessellation::BuildCrateGeometryBuffers()
{
    GeometryGenerator::MeshData box;
    GeometryGenerator geoGen;

    geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);

    std::vector<Vertex::Basic32> vertices(box.Vertices.size());
    for (UINT i = 0; i < box.Vertices.size(); ++i)
    {
        vertices[i].Pos = box.Vertices[i].Position;
        vertices[i].Normal = box.Vertices[i].Normal;
        vertices[i].Tex = box.Vertices[i].TexC;
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex::Basic32) * box.Vertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];

    HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mBoxVB));

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * box.Indices.size();
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &box.Indices[0];
    HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mBoxIB));
}

void BasicTessellation::BuildScreenQuadGeometryBuffers()
{
    GeometryGenerator::MeshData quad;

    GeometryGenerator geoGen;

    geoGen.CreateFullscreenQuad(quad);

    std::vector<Vertex::Basic32> vertices(quad.Vertices.size());

    for (UINT i = 0; i < quad.Vertices.size(); ++i)
    {
        vertices[i].Pos    = quad.Vertices[i].Position;
        vertices[i].Normal = quad.Vertices[i].Normal;
        vertices[i].Tex    = quad.Vertices[i].TexC;
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex::Basic32) * quad.Vertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mScreenQuadVB));

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * quad.Indices.size();
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &quad.Indices[0];
    HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mScreenQuadIB));
}

void BasicTessellation::BuildOffscreenViews()
{
    ReleaseCOM(mOffscreenSRV);
    ReleaseCOM(mOffscreenRTV);
    ReleaseCOM(mOffscreenUAV);

    D3D11_TEXTURE2D_DESC texDesc;
    texDesc.Width = mClientWidth;
    texDesc.Height = mClientHeight;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_RENDER_TARGET;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;

    ID3D11Texture2D* offscreenTex = 0;
    HR(md3dDevice->CreateTexture2D(&texDesc, 0, &offscreenTex));

    HR(md3dDevice->CreateShaderResourceView(offscreenTex, 0, &mOffscreenSRV));
    HR(md3dDevice->CreateUnorderedAccessView(offscreenTex, 0, &mOffscreenUAV));
    HR(md3dDevice->CreateRenderTargetView(offscreenTex, 0, &mOffscreenRTV));

    ReleaseCOM(offscreenTex);
}
