#include "Skull.h"

Skull::Skull(HINSTANCE hInstance)
    : D3DApp(hInstance),
    mVertexBuffer(0), mIndexBuffer(0),
    mFX(0), mTech(0), mfxWorldViewProj(0),
    mInputLayout(0), mWireFrameRS(0), mSkullIndexCount(0),
    mTheta(1.5f * MathHelper::Pi), mPhi(0.1f * MathHelper::Pi), mRadius(20.0f)
{
    mMainWndCaption = L"Skull";
    
    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    XMMATRIX I = XMMatrixIdentity();
    XMStoreFloat4x4(&mView, I);
    XMStoreFloat4x4(&mProj, I);

    XMMATRIX T = XMMatrixTranslation(0.0f, -2.0f, 0.0f);
    XMStoreFloat4x4(&mSkullWorld, T);
}

Skull::~Skull()
{
    ReleaseCOM(mVertexBuffer);
    ReleaseCOM(mIndexBuffer);
    ReleaseCOM(mFX);
    ReleaseCOM(mInputLayout);
    ReleaseCOM(mWireFrameRS);
}

bool Skull::Init()
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

void Skull::OnResize()
{
    D3DApp::OnResize();

    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj, P);
}

void Skull::UpdateScene(float dt)
{
    float x = mRadius * sinf(mPhi) * cosf(mTheta);
    float z = mRadius * sinf(mPhi) * sinf(mTheta);
    float y = mRadius * cosf(mPhi);

    XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
    XMVECTOR target = XMVectorZero();
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
    XMStoreFloat4x4(&mView, V);
}

void Skull::DrawScene()
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

    XMMATRIX view  = XMLoadFloat4x4(&mView);
    XMMATRIX proj  = XMLoadFloat4x4(&mProj);
    XMMATRIX world = XMLoadFloat4x4(&mSkullWorld);
    XMMATRIX worldViewProj = world * view * proj;

    mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));

    D3DX11_TECHNIQUE_DESC techDesc;
    mTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);
    }

    HR(mSwapChain->Present(0, 0));
}


void Skull::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void Skull::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void Skull::OnMouseMove(WPARAM btnState, int x, int y)
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
        float dx = 0.05f * static_cast<float>(x - mLastMousePos.x);
        float dy = 0.05f * static_cast<float>(y - mLastMousePos.y);

        // Update the camera radius based on input.
        mRadius += dx - dy;

        // Restrict the radius.
        mRadius = MathHelper::Clamp(mRadius, 5.0f, 50.0f);
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}

void Skull::BuildGeometryBuffers()
{
    std::ifstream fin("models/skull.txt");
    
    if (!fin)
    {
        MessageBox(0, L"models/skull.txt not found", 0, 0);
        return;
    }

    UINT vCount = 0;
    UINT tCount = 0;
    std::string ignore;

    fin >> ignore >> vCount;
    fin >> ignore >> tCount;
    fin >> ignore >> ignore >> ignore >> ignore;

    float nx, ny, nz;
    XMFLOAT4 black(0.0f, 0.0f, 0.0f, 0.0f);

    std::vector<Vertex> vertices(vCount);
    for (UINT i = 0; i < vCount; ++i)
    {
        fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
        
        vertices[i].Color = black;

        fin >> nx >> ny >> nz;
    }

    fin >> ignore >> ignore >> ignore;

    mSkullIndexCount = 3 * tCount;
    std::vector<UINT> indices(mSkullIndexCount);
    for (UINT i = 0; i < tCount; ++i)
    {
        fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
    }

    fin.close();

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex) * vCount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA vInitData;
    vInitData.pSysMem = &vertices[0];
    
    HR(md3dDevice->CreateBuffer(&vbd, &vInitData, &mVertexBuffer));

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * mSkullIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA iInitData;
    iInitData.pSysMem = &indices[0];

    HR(md3dDevice->CreateBuffer(&ibd, &iInitData, &mIndexBuffer));
}

void Skull::BuildFX()
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

void Skull::BuildVertexLayout()
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