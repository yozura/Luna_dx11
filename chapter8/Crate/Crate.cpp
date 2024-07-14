#include "Crate.h"

using namespace DirectX;

Crate::Crate(HINSTANCE hInstance)
    : D3DApp(hInstance), mBoxVertexBuffer(0), mBoxIndexBuffer(0), mDiffuseMapSRV(0)
    , mEyePosW(0.0f, 0.0f, 0.0f), mTheta(1.3f * MathHelper::Pi), mPhi(0.1f * MathHelper::Pi), mRadius(2.0f)
{
    mMainWndCaption = L"Crate";

    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    XMMATRIX I = XMMatrixIdentity();
    XMStoreFloat4x4(&mBoxWorld, I);
    XMStoreFloat4x4(&mTexTransform, I);
    XMStoreFloat4x4(&mView, I);
    XMStoreFloat4x4(&mProj, I);

    mDirLights[0].Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    mDirLights[0].Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    mDirLights[0].Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
    mDirLights[0].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

    mDirLights[1].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirLights[1].Diffuse = XMFLOAT4(1.4f, 1.4f, 1.4f, 1.0f);
    mDirLights[1].Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 16.0f);
    mDirLights[1].Direction = XMFLOAT3(-0.707f, 0.0f, 0.707f);

    mBoxMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    mBoxMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mBoxMat.Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);

    XMMATRIX boxTexScale = XMMatrixScaling(3.0f, 3.0f, 3.0f);
    XMStoreFloat4x4(&mTexTransform, boxTexScale);
}

Crate::~Crate()
{
    ReleaseCOM(mBoxVertexBuffer);
    ReleaseCOM(mBoxIndexBuffer);
    ReleaseCOM(mDiffuseMapSRV);

    Effects::DestroyAll();
    InputLayouts::DestroyAll();
}

bool Crate::Init()
{
    if (!D3DApp::Init())
        return false;

    Effects::InitAll(md3dDevice);
    InputLayouts::InitAll(md3dDevice);

    ScratchImage texture;
    HR(LoadFromDDSFile(L"textures/mipmaps.dds", DDS_FLAGS_NONE, nullptr, texture));
    HR(CreateShaderResourceView(md3dDevice, texture.GetImages(), texture.GetImageCount(), texture.GetMetadata(), &mDiffuseMapSRV));

    BuildGeometryBuffers();

    return true;
}

void Crate::OnResize()
{
    D3DApp::OnResize();

    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj, P);
}

void Crate::UpdateScene(float dt)
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
}

void Crate::DrawScene()
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

    ID3DX11EffectTechnique* activeTech = Effects::BasicFX->Light2TexTech;

    D3DX11_TECHNIQUE_DESC techDesc;
    activeTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxVertexBuffer, &stride, &offset);
        md3dImmediateContext->IASetIndexBuffer(mBoxIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

        // Box
        XMMATRIX world = XMLoadFloat4x4(&mBoxWorld);
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
        XMMATRIX worldViewProj = world * view * proj;

        Effects::BasicFX->SetWorld(world);
        Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::BasicFX->SetWorldViewProj(worldViewProj);
        Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&mTexTransform));
        Effects::BasicFX->SetMaterial(mBoxMat);
        Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRV);

        activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);
    }

    HR(mSwapChain->Present(0, 0));
}

void Crate::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void Crate::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void Crate::OnMouseMove(WPARAM btnState, int x, int y)
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
        mRadius = MathHelper::Clamp(mRadius, 1.0f, 15.0f);
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}

void Crate::BuildGeometryBuffers()
{
    GeometryGenerator::MeshData box;
    GeometryGenerator geoGen;

    geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);

    mBoxVertexOffset = 0;
    mBoxIndexOffset = 0;
    mBoxIndexCount = box.Indices.size();

    UINT totalVertexCount = box.Vertices.size();
    UINT totalIndexCount  = mBoxIndexCount;

    std::vector<Vertex::Basic32> vertices(totalVertexCount);
    UINT k = 0;
    for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos    = box.Vertices[i].Position;
        vertices[k].Normal = box.Vertices[i].Normal;
        vertices[k].Tex    = box.Vertices[i].TexC;
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex::Basic32) * totalVertexCount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA vInitData;
    vInitData.pSysMem = &vertices[0];

    HR(md3dDevice->CreateBuffer(&vbd, &vInitData, &mBoxVertexBuffer));

    std::vector<UINT> indices;
    indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA iInitData;
    iInitData.pSysMem = &indices[0];

    HR(md3dDevice->CreateBuffer(&ibd, &iInitData, &mBoxIndexBuffer));
}
