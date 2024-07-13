#include "TexturedHillsAndWaves.h"

using namespace DirectX;

TexturedHillsAndWaves::TexturedHillsAndWaves(HINSTANCE hInstance)
    : D3DApp(hInstance), mLandVertexBuffer(0), mWavesVertexBuffer(0), mLandIndexBuffer(0), mWavesIndexBuffer(0)
    , mLandIndexCount(0), mEyePosW(0.0f, 0.0f, 0.0f), mWaterTexOffset(0.0f, 0.0f), mGrassMapSRV(0), mWaterMapSRV(0)
    , mTheta(1.3f * MathHelper::Pi), mPhi(0.4f * MathHelper::Pi), mRadius(120.0f)
{
    mMainWndCaption = L"TexturedHillsAndWaves";

    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    XMMATRIX I = XMMatrixIdentity();
    XMStoreFloat4x4(&mLandWorld, I);
    XMStoreFloat4x4(&mWavesWorld, I);
    XMStoreFloat4x4(&mView, I);
    XMStoreFloat4x4(&mProj, I);

    XMMATRIX grassTexScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);
    XMStoreFloat4x4(&mGrassTexTransform, grassTexScale);

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

    mLandMat.Ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    mLandMat.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mLandMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

    mWavesMat.Ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    mWavesMat.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mWavesMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f);
}

TexturedHillsAndWaves::~TexturedHillsAndWaves()
{
    ReleaseCOM(mLandVertexBuffer);
    ReleaseCOM(mLandIndexBuffer);
    ReleaseCOM(mWavesVertexBuffer);
    ReleaseCOM(mWavesIndexBuffer);
    ReleaseCOM(mGrassMapSRV);
    ReleaseCOM(mWaterMapSRV);

    Effects::DestroyAll();
    InputLayouts::DestroyAll();
}

bool TexturedHillsAndWaves::Init()
{
    if (!D3DApp::Init())
        return false;

    mWaves.Init(160, 160, 1.0f, 0.03f, 3.25f, 0.4f);

    Effects::InitAll(md3dDevice);
    InputLayouts::InitAll(md3dDevice);

    ScratchImage grass;
    HR(LoadFromDDSFile(L"textures/grass.dds", DDS_FLAGS_NONE, nullptr, grass));
    HR(CreateShaderResourceView(md3dDevice, grass.GetImages(), grass.GetImageCount(), grass.GetMetadata(), &mGrassMapSRV));

    ScratchImage water;
    HR(LoadFromDDSFile(L"textures/water2.dds", DDS_FLAGS_NONE, nullptr, water));
    HR(CreateShaderResourceView(md3dDevice, water.GetImages(), water.GetImageCount(), water.GetMetadata(), &mWaterMapSRV));

    BuildLandGeometryBuffers();
    BuildWaveGeometryBuffers();

    return true;
}

void TexturedHillsAndWaves::OnResize()
{
    D3DApp::OnResize();

    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj, P);
}

void TexturedHillsAndWaves::UpdateScene(float dt)
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
    if ((mTimer.TotalTime() - t_base) >= 0.25f)
    {
        t_base += 0.25f;

        DWORD i = 5 + rand() % (mWaves.RowCount() - 10);
        DWORD j = 5 + rand() % (mWaves.ColumnCount() - 10);
        float r = MathHelper::RandF(1.0f, 2.0f);

        mWaves.Disturb(i, j, r);
    }

    mWaves.Update(dt);
    
    D3D11_MAPPED_SUBRESOURCE mappedData;
    md3dImmediateContext->Map(mWavesVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
    
    Vertex::Basic32* v = reinterpret_cast<Vertex::Basic32*>(mappedData.pData);
    for (UINT i = 0; i < mWaves.VertexCount(); ++i)
    {
        v[i].Pos    = mWaves[i];
        v[i].Normal = mWaves.Normal(i);

        v[i].Tex.x  = 0.5f + mWaves[i].x / mWaves.Width();
        v[i].Tex.y  = 0.5f - mWaves[i].z / mWaves.Depth();
    }

    md3dImmediateContext->Unmap(mWavesVertexBuffer, 0);

    XMMATRIX wavesScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);

    mWaterTexOffset.y = 0.05f * dt;
    mWaterTexOffset.x = 0.1f  * dt;

    XMMATRIX wavesOffset = XMMatrixTranslation(mWaterTexOffset.x, mWaterTexOffset.y, 0.0f);

    XMStoreFloat4x4(&mWaterTexTransform, wavesScale * wavesOffset);
}

void TexturedHillsAndWaves::DrawScene()
{
    md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Aquamarine));
    md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    
    md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    UINT stride = sizeof(Vertex::Basic32);
    UINT offset = 0;

    XMMATRIX view = XMLoadFloat4x4(&mView);
    XMMATRIX proj = XMLoadFloat4x4(&mProj);
    
    Effects::BasicFX->SetDirLights(mDirLights);
    Effects::BasicFX->SetEyePosW(mEyePosW);

    ID3DX11EffectTechnique* activeTech = Effects::BasicFX->Light3TexTech;

    D3DX11_TECHNIQUE_DESC techDesc;
    activeTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        md3dImmediateContext->IASetVertexBuffers(0, 1, &mLandVertexBuffer, &stride, &offset);
        md3dImmediateContext->IASetIndexBuffer(mLandIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

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

        activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexed(mLandIndexCount, 0, 0);

        // Waves
        md3dImmediateContext->IASetVertexBuffers(0, 1, &mWavesVertexBuffer, &stride, &offset);
        md3dImmediateContext->IASetIndexBuffer(mWavesIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

        world = XMLoadFloat4x4(&mWavesWorld);
        worldInvTranspose = MathHelper::InverseTranspose(world);
        worldViewProj = world * view * proj;

        Effects::BasicFX->SetWorld(world);
        Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::BasicFX->SetWorldViewProj(worldViewProj);
        Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&mWaterTexTransform));
        Effects::BasicFX->SetMaterial(mWavesMat);
        Effects::BasicFX->SetDiffuseMap(mWaterMapSRV);

        activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexed(3 * mWaves.TriangleCount(), 0, 0);
    }

    HR(mSwapChain->Present(0, 0));
}

void TexturedHillsAndWaves::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void TexturedHillsAndWaves::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void TexturedHillsAndWaves::OnMouseMove(WPARAM btnState, int x, int y)
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
        float dx = 0.05f * static_cast<float>(x - mLastMousePos.x);
        float dy = 0.05f * static_cast<float>(y - mLastMousePos.y);

        // Update the camera radius based on input.
        mRadius += dx - dy;

        // Restrict the radius.
        mRadius = MathHelper::Clamp(mRadius, 50.0f, 500.0f);
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}

float TexturedHillsAndWaves::GetHillHeight(float x, float z) const
{
    return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}

XMFLOAT3 TexturedHillsAndWaves::GetHillNormal(float x, float z) const
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

void TexturedHillsAndWaves::BuildLandGeometryBuffers()
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

    HR(md3dDevice->CreateBuffer(&vbd, &vInitData, &mLandVertexBuffer));

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * mLandIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA iInitData;
    iInitData.pSysMem = &grid.Indices[0];

    HR(md3dDevice->CreateBuffer(&ibd, &iInitData, &mLandIndexBuffer));
}

void TexturedHillsAndWaves::BuildWaveGeometryBuffers()
{
    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_DYNAMIC;
    vbd.ByteWidth = sizeof(Vertex::Basic32) * mWaves.VertexCount();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vbd.MiscFlags = 0;

    HR(md3dDevice->CreateBuffer(&vbd, 0, &mWavesVertexBuffer));

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
    HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mWavesIndexBuffer));
}
