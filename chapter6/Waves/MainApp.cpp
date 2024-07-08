#include "MainApp.h"

MainApp::MainApp(HINSTANCE hInstance)
    : D3DApp(hInstance), mLandVertexBuffer(0), mLandIndexBuffer(0)
    , mWavesVertexBuffer(0), mWavesIndexBuffer(0), mFX(0), mTech(0)
    , mfxWorldViewProj(0), mInputLayout(0), mWireFrameRS(0)
    , mGridIndexCount(0), mTheta(1.5f * MathHelper::Pi)
    , mPhi(0.1f * MathHelper::Pi), mRadius(200.0f)
{
    mMainWndCaption = L"Waves";

    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    XMMATRIX I = XMMatrixIdentity();
    XMStoreFloat4x4(&mGridWorld, I);
    XMStoreFloat4x4(&mWavesWorld, I);
    XMStoreFloat4x4(&mView, I);
    XMStoreFloat4x4(&mProj, I);
}

MainApp::~MainApp()
{
    ReleaseCOM(mLandVertexBuffer);
    ReleaseCOM(mLandIndexBuffer);
    ReleaseCOM(mWavesVertexBuffer);
    ReleaseCOM(mWavesIndexBuffer);
    ReleaseCOM(mFX);
    ReleaseCOM(mInputLayout);
    ReleaseCOM(mWireFrameRS);
}

bool MainApp::Init()
{
    if (!D3DApp::Init())
        return false;

    mWaves.Init(200, 200, 0.8f, 0.03f, 3.25f, 0.4f);

    BuildLandGeometryBuffers();
    BuildWavesGeometryBuffers();
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

void MainApp::OnResize()
{
    D3DApp::OnResize();

    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj, P);
}

void MainApp::UpdateScene(float dt)
{
    float x = mRadius * sinf(mPhi) * cosf(mTheta);
    float z = mRadius * sinf(mPhi) * sinf(mTheta);
    float y = mRadius * cosf(mPhi);

    XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
    XMVECTOR target = XMVectorZero();
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
    XMStoreFloat4x4(&mView, V);

    // 매 0.25초마다 랜덤 웨이브 생성
    static float t_base = 0.0f;
    if ((mTimer.TotalTime() - t_base) >= 0.25f)
    {
        t_base += 0.25f;

        DWORD i = 5 + rand() % 190;
        DWORD j = 5 + rand() % 190;

        float r = MathHelper::RandF(1.0f, 2.0f);

        mWaves.Disturb(i, j, r);
    }

    mWaves.Update(dt);

    D3D11_MAPPED_SUBRESOURCE mappedData;
    HR(md3dImmediateContext->Map(mWavesVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

    Vertex* v = reinterpret_cast<Vertex*>(mappedData.pData);
    for (UINT i = 0; i < mWaves.VertexCount(); ++i)
    {
        v[i].Pos   = mWaves[i];
        v[i].Color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    }

    md3dImmediateContext->Unmap(mWavesVertexBuffer, 0);
}

void MainApp::DrawScene()
{
    md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
    md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    md3dImmediateContext->IASetInputLayout(mInputLayout);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    XMMATRIX view = XMLoadFloat4x4(&mView);
    XMMATRIX proj = XMLoadFloat4x4(&mProj);

    D3DX11_TECHNIQUE_DESC techDesc;
    mTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        // 지형 그리기
        md3dImmediateContext->IASetVertexBuffers(0, 1, &mLandVertexBuffer, &stride, &offset);
        md3dImmediateContext->IASetIndexBuffer(mLandIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

        XMMATRIX world = XMLoadFloat4x4(&mGridWorld);
        XMMATRIX worldViewProj = world * view * proj;

        mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
        mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexed(mGridIndexCount, 0, 0);

        // 파도 그리기
        md3dImmediateContext->RSSetState(mWireFrameRS);

        md3dImmediateContext->IASetVertexBuffers(0, 1, &mWavesVertexBuffer, &stride, &offset);
        md3dImmediateContext->IASetIndexBuffer(mWavesIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

        world = XMLoadFloat4x4(&mWavesWorld);
        worldViewProj = world * view * proj;

        mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
        mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexed(3 * mWaves.TriangleCount(), 0, 0);

        md3dImmediateContext->RSSetState(0);
    }

    HR(mSwapChain->Present(0, 0));
}


void MainApp::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void MainApp::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void MainApp::OnMouseMove(WPARAM btnState, int x, int y)
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
        // Make each pixel correspond to 0.2 unit in the scene.
        float dx = 0.2f * static_cast<float>(x - mLastMousePos.x);
        float dy = 0.2f * static_cast<float>(y - mLastMousePos.y);

        // Update the camera radius based on input.
        mRadius += dx - dy;

        // Restrict the radius.
        mRadius = MathHelper::Clamp(mRadius, 50.0f, 500.0f);
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}

float MainApp::GetHeight(float x, float z) const
{
    return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}

void MainApp::BuildLandGeometryBuffers()
{
    GeometryGenerator::MeshData grid;

    GeometryGenerator geoGen;

    geoGen.CreateGrid(160.0f, 160.0f, 50, 50, grid);

    mGridIndexCount = grid.Indices.size();

    std::vector<Vertex> vertices(grid.Vertices.size());
    for (size_t i = 0; i < grid.Vertices.size(); ++i)
    {
        XMFLOAT3 p = grid.Vertices[i].Position;
        p.y = GetHeight(p.x, p.z);

        vertices[i].Pos = p;
        
        // Color the vertex based on its height.
        if (p.y < -10.0f)
        {
            // Sandy beach color.
            vertices[i].Color = XMFLOAT4(1.0f, 0.96f, 0.62f, 1.0f);
        }
        else if (p.y < 5.0f)
        {
            // Light yellow-green.
            vertices[i].Color = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
        }
        else if (p.y < 12.0f)
        {
            // Dark yellow-green.
            vertices[i].Color = XMFLOAT4(0.1f, 0.48f, 0.19f, 1.0f);
        }
        else if (p.y < 20.0f)
        {
            // Dark brown.
            vertices[i].Color = XMFLOAT4(0.45f, 0.39f, 0.34f, 1.0f);
        }
        else
        {
            // White snow.
            vertices[i].Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        }
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.ByteWidth = sizeof(Vertex) * grid.Vertices.size();
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA vInitData;
    vInitData.pSysMem = &vertices[0];

    HR(md3dDevice->CreateBuffer(&vbd, &vInitData, &mLandVertexBuffer));

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.ByteWidth = sizeof(UINT) * mGridIndexCount;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA iInitData;
    iInitData.pSysMem = &grid.Indices[0];

    HR(md3dDevice->CreateBuffer(&ibd, &iInitData, &mLandIndexBuffer));
}

void MainApp::BuildWavesGeometryBuffers()
{
    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_DYNAMIC;
    vbd.ByteWidth = sizeof(Vertex) * mWaves.VertexCount();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
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
            indices[k]     = i * n + j;
            indices[k + 1] = i * n + j + 1;
            indices[k + 2] = (i * 1) * n + j;
            
            indices[k + 3] = (i + 1) * n + j;
            indices[k + 4] = i * n + j + 1;
            indices[k + 5] = (i + 1) * n + j + 1;

            k += 6;
        }
    }

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * indices.size();
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    
    D3D11_SUBRESOURCE_DATA iInitData;
    iInitData.pSysMem = &indices[0];

    HR(md3dDevice->CreateBuffer(&ibd, &iInitData, &mWavesIndexBuffer));
}

void MainApp::BuildFX()
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

void MainApp::BuildVertexLayout()
{
    // 정점 셰이더 정보
    D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    // 입력 레이아웃 생성
    D3DX11_PASS_DESC passDesc;
    mTech->GetPassByIndex(0)->GetDesc(&passDesc);
    HR(md3dDevice->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mInputLayout));
}
