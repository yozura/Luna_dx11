#include "InstancingAndCulling.h"

using namespace DirectX;

InstancingAndCulling::InstancingAndCulling(HINSTANCE hInstance)
    : D3DApp(hInstance), mSkullVB(0), mSkullIB(0)
    , mSkullIndexCount(0), mInstancedBuffer(0)
    , mVisibleObjectCount(0), mFrustumCullingEnabled(true)
{
    mMainWndCaption = L"Instancing and Culling";

    srand((unsigned int)time((time_t*)NULL));

    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    mCam.SetPosition(0.0f, 2.0f, -15.0f);

    XMMATRIX skullScale  = XMMatrixScaling(0.5f, 0.5f, 0.5f);
    XMMATRIX skullOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
    XMStoreFloat4x4(&mSkullWorld, XMMatrixMultiply(skullScale, skullOffset));

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

    mSkullMat.Ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
    mSkullMat.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    mSkullMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
}

InstancingAndCulling::~InstancingAndCulling()
{
    ReleaseCOM(mSkullVB);
    ReleaseCOM(mSkullIB);
    ReleaseCOM(mInstancedBuffer);

    Effects::DestroyAll();
    InputLayouts::DestroyAll();
}

bool InstancingAndCulling::Init()
{
    if (!D3DApp::Init())
        return false;

    Effects::InitAll(md3dDevice);
    InputLayouts::InitAll(md3dDevice);

    BuildSkullGeometryBuffers();
    BuildInstancedBuffer();

    return true;
}

void InstancingAndCulling::OnResize()
{
    D3DApp::OnResize();

    mCam.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);

    BoundingFrustum::CreateFromMatrix(mCamFrustum, mCam.Proj());
}

void InstancingAndCulling::UpdateScene(float dt)
{
    if (GetAsyncKeyState('W') & 0x8000)
        mCam.Walk(10.0f * dt);
    if (GetAsyncKeyState('S') & 0x8000)
        mCam.Walk(-10.0f * dt);
    if (GetAsyncKeyState('A') & 0x8000)
        mCam.Strafe(-10.0f * dt);
    if (GetAsyncKeyState('D') & 0x8000)
        mCam.Strafe(10.0f * dt);

    if (GetAsyncKeyState('1') & 0x8000)
        mFrustumCullingEnabled = true;
    if (GetAsyncKeyState('2') & 0x8000)
        mFrustumCullingEnabled = false;

    mCam.UpdateViewMatrix();

    mVisibleObjectCount = 0;

    if (mFrustumCullingEnabled)
    {
        XMVECTOR detView = XMMatrixDeterminant(mCam.View());
        XMMATRIX invView = XMMatrixInverse(&detView, mCam.View());

        D3D11_MAPPED_SUBRESOURCE mappedData;
        md3dImmediateContext->Map(mInstancedBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);

        InstancedData* dataView = reinterpret_cast<InstancedData*>(mappedData.pData);

        for (UINT i = 0; i < mInstancedData.size(); ++i)
        {
            XMMATRIX W    = XMLoadFloat4x4(&mInstancedData[i].World);
            XMVECTOR detW = XMMatrixDeterminant(W);
            XMMATRIX invWorld = XMMatrixInverse(&detW, W);

            // View space to the object's local space.
            XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);

            // Decompose the matrix into its individual parts.
            XMVECTOR scale, rotQuat, translation;
            XMMatrixDecompose(&scale, &rotQuat, &translation, toLocal);

            BoundingFrustum localSpaceFrustum;
            mCamFrustum.Transform(localSpaceFrustum, XMVectorGetX(scale), rotQuat, translation);
            
            if (localSpaceFrustum.Intersects(mSkullBox))
            {
                dataView[mVisibleObjectCount++] = mInstancedData[i];
            }
        }

        md3dImmediateContext->Unmap(mInstancedBuffer, 0);
    }
    else
    {
        D3D11_MAPPED_SUBRESOURCE mappedData;
        md3dImmediateContext->Map(mInstancedBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
        
        InstancedData* dataView = reinterpret_cast<InstancedData*>(mappedData.pData);

        for (UINT i = 0; i < mInstancedData.size(); ++i)
        {
            dataView[mVisibleObjectCount++] = mInstancedData[i];
        }

        md3dImmediateContext->Unmap(mInstancedBuffer, 0);
    }

    std::wostringstream outs;
    outs.precision(6);
    outs << L"Instancing and Culling: "
         << mVisibleObjectCount
         << L" objects visible out of "
         << mInstancedData.size();
    mMainWndCaption = outs.str();
}

void InstancingAndCulling::DrawScene()
{
    md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
    md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    md3dImmediateContext->IASetInputLayout(InputLayouts::InstancedBasic32);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    UINT stride[2] = { sizeof(Vertex::Basic32), sizeof(InstancedData) };
    UINT offset[2] = { 0, 0 };

    ID3D11Buffer* vbs[2] = { mSkullVB, mInstancedBuffer };

    XMMATRIX view     = mCam.View();
    XMMATRIX proj     = mCam.Proj();
    XMMATRIX viewProj = mCam.ViewProj();
    
    Effects::InstancedBasicFX->SetDirLights(mDirLights);
    Effects::InstancedBasicFX->SetEyePosW(mCam.GetPosition());

    ID3DX11EffectTechnique* activeTech = Effects::InstancedBasicFX->Light3Tech;

    D3DX11_TECHNIQUE_DESC techDesc;
    activeTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        md3dImmediateContext->IASetVertexBuffers(0, 2, vbs, stride, offset);
        md3dImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);

        XMMATRIX world = XMLoadFloat4x4(&mSkullWorld);
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);

        Effects::InstancedBasicFX->SetWorld(world);
        Effects::InstancedBasicFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::InstancedBasicFX->SetViewProj(viewProj);
        Effects::InstancedBasicFX->SetMaterial(mSkullMat);

        activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexedInstanced(mSkullIndexCount, mVisibleObjectCount, 0, 0, 0);
    }

    HR(mSwapChain->Present(0, 0));
}


void InstancingAndCulling::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void InstancingAndCulling::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void InstancingAndCulling::OnMouseMove(WPARAM btnState, int x, int y)
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

void InstancingAndCulling::BuildInstancedBuffer()
{
    const int n = 5;
    mInstancedData.resize(n * n * n);

    float width  = 200.0f;
    float height = 200.0f;
    float depth  = 200.0f;

    float x = -0.5f * width;
    float y = -0.5f * height;
    float z = -0.5f * depth;

    float dx = width / (n - 1);
    float dy = height / (n - 1);
    float dz = depth / (n - 1);

    for (int k = 0; k < n; ++k)
    {
        for (int i = 0; i < n; ++i)
        {
            for (int j = 0; j < n; ++j)
            {
                mInstancedData[k * n * n + i * n + j].World = XMFLOAT4X4(
                    1.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f,
                    x + j * dx, y + i * dy, z + k * dz, 1.0f);

                mInstancedData[k * n * n + i * n + j].Color.x = MathHelper::RandF(0.0f, 1.0f);
                mInstancedData[k * n * n + i * n + j].Color.y = MathHelper::RandF(0.0f, 1.0f);
                mInstancedData[k * n * n + i * n + j].Color.z = MathHelper::RandF(0.0f, 1.0f);
                mInstancedData[k * n * n + i * n + j].Color.w = 1.0f;
            }
        }
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_DYNAMIC;
    vbd.ByteWidth = sizeof(InstancedData) * mInstancedData.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vbd.MiscFlags = 0;
    vbd.StructureByteStride = 0;

    HR(md3dDevice->CreateBuffer(&vbd, 0, &mInstancedBuffer));
}

void InstancingAndCulling::BuildSkullGeometryBuffers()
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