#include "Shapes.h"

Shapes::Shapes(HINSTANCE hInstance)
    : D3DApp(hInstance),
    mVertexBuffer(0), mIndexBuffer(0),
    mFX(0), mTech(0), mfxWorldViewProj(0),
    mInputLayout(0), mWireFrameRS(0),
    mTheta(1.5f * MathHelper::Pi),
    mPhi(0.1f * MathHelper::Pi),
    mRadius(15.0f)
{
    mMainWndCaption = L"Shapes";

    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    XMMATRIX I = XMMatrixIdentity();
    XMStoreFloat4x4(&mGridWorld, I);
    XMStoreFloat4x4(&mView, I);
    XMStoreFloat4x4(&mProj, I);

    XMMATRIX boxScale = XMMatrixScaling(2.0f, 1.0f, 2.0f);
    XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
    XMStoreFloat4x4(&mBoxWorld, XMMatrixMultiply(boxScale, boxOffset));

    XMMATRIX centerSphereScale = XMMatrixScaling(2.0f, 2.0f, 2.0f);
    XMMATRIX centerSphereOffset = XMMatrixTranslation(0.0f, 2.0f, 0.0f);
    XMStoreFloat4x4(&mCenterSphere, XMMatrixMultiply(centerSphereScale, centerSphereOffset));

    for (int i = 0; i < 5; ++i)
    {
        XMStoreFloat4x4(&mCylinderWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f));
        XMStoreFloat4x4(&mCylinderWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f));

        XMStoreFloat4x4(&mSphereWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f));
        XMStoreFloat4x4(&mSphereWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f));
    }
}

Shapes::~Shapes()
{
    ReleaseCOM(mVertexBuffer);
    ReleaseCOM(mIndexBuffer);
    ReleaseCOM(mFX);
    ReleaseCOM(mInputLayout);
    ReleaseCOM(mWireFrameRS);
}

bool Shapes::Init()
{
    if (!D3DApp::Init())
        return false;

    BuildGeometryBuffers();
    BuildFX();
    BuildVertexLayout();

    D3D11_RASTERIZER_DESC wireFrameDesc;
    ZeroMemory(&wireFrameDesc, sizeof(D3D11_RASTERIZER_DESC));
    wireFrameDesc.FillMode = D3D11_FILL_WIREFRAME;
    wireFrameDesc.CullMode = D3D11_CULL_BACK;
    wireFrameDesc.FrontCounterClockwise = false;
    wireFrameDesc.DepthClipEnable = true;

    HR(md3dDevice->CreateRasterizerState(&wireFrameDesc, &mWireFrameRS));

    return true;
}

void Shapes::OnResize()
{
    D3DApp::OnResize();

    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj, P);
}

void Shapes::UpdateScene(float dt)
{
    float x = mRadius * sinf(mPhi) * cosf(mTheta);
    float z = mRadius * sinf(mPhi) * sinf(mTheta);
    float y = mRadius * cosf(mPhi);

    XMVECTOR pos    = XMVectorSet(x, y, z, 1.0f);
    XMVECTOR target = XMVectorZero();
    XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
    XMStoreFloat4x4(&mView, V);
}

void Shapes::DrawScene()
{
    md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
    md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    md3dImmediateContext->IASetInputLayout(mInputLayout);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    md3dImmediateContext->RSSetState(mWireFrameRS);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    md3dImmediateContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
    md3dImmediateContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    XMMATRIX view = XMLoadFloat4x4(&mView);
    XMMATRIX proj = XMLoadFloat4x4(&mProj);
    XMMATRIX viewProj = view * proj;

    D3DX11_TECHNIQUE_DESC techDesc;
    mTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        // 그리드
        XMMATRIX world = XMLoadFloat4x4(&mGridWorld);
        XMMATRIX worldViewProj = world * viewProj;
        mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
        mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

        // 박스
        world = XMLoadFloat4x4(&mBoxWorld);
        worldViewProj = world * viewProj;
        mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
        mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

        // 중심 구
        world = XMLoadFloat4x4(&mCenterSphere);
        worldViewProj = world * viewProj;
        mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
        mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);

        // 원기둥
        for (int i = 0; i < 10; ++i)
        {
            world = XMLoadFloat4x4(&mCylinderWorld[i]);
            worldViewProj = world * viewProj;
            mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
            mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
            md3dImmediateContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
        }

        // 구
        for (int i = 0; i < 10; ++i)
        {
            world = XMLoadFloat4x4(&mSphereWorld[i]);
            worldViewProj = world * viewProj;
            mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
            mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
            md3dImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
        }
    }

    HR(mSwapChain->Present(0, 0));
}


void Shapes::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void Shapes::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void Shapes::OnMouseMove(WPARAM btnState, int x, int y)
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
        mRadius = MathHelper::Clamp(mRadius, 3.0f, 200.0f);
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}

void Shapes::BuildGeometryBuffers()
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
    mGridIndexOffset     = mBoxIndexCount;
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

    std::vector<Vertex> vertices(totalVertexCount);
    XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);
    
    UINT k = 0;
    for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos   = box.Vertices[i].Position;
        vertices[k].Color = XMFLOAT4(Colors::Orchid);
    }
    
    for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos   = grid.Vertices[i].Position;
        vertices[k].Color = XMFLOAT4(Colors::LightGray);
    }

    for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos   = sphere.Vertices[i].Position;
        vertices[k].Color = XMFLOAT4(Colors::LightGoldenrodYellow);
    }

    for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos   = cylinder.Vertices[i].Position;
        vertices[k].Color = XMFLOAT4(Colors::OliveDrab);
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex) * totalVertexCount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA vInitData;
    vInitData.pSysMem = &vertices[0];

    HR(md3dDevice->CreateBuffer(&vbd, &vInitData, &mVertexBuffer));

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
    
    HR(md3dDevice->CreateBuffer(&ibd, &iInitData, &mIndexBuffer));
}

void Shapes::BuildFX()
{
    std::ifstream fin("shaders/color.cso", std::ios::binary);

    fin.seekg(0, std::ios_base::end);
    int size = (int)fin.tellg();
    fin.seekg(0, std::ios_base::beg);
    std::vector<char> compiledShader(size);

    fin.read(&compiledShader[0], size);
    fin.close();

    HR(D3DX11CreateEffectFromMemory(&compiledShader[0], size, 0, md3dDevice, &mFX));

    mTech = mFX->GetTechniqueByName("ColorTech");
    mfxWorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
}

void Shapes::BuildVertexLayout()
{
    D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    D3DX11_PASS_DESC passDesc;
    mTech->GetPassByIndex(0)->GetDesc(&passDesc);
    HR(md3dDevice->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mInputLayout));
}