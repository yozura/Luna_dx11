#include "PickingApp.h"

using namespace DirectX;

PickingApp::PickingApp(HINSTANCE hInstance)
    : D3DApp(hInstance)
    , mMeshVB(0), mMeshIB(0)
    , mBoundingBoxVB(0), mBoundingBoxIB(0)
    , mMeshIndexCount(0), mPickedTriangle(-1)
{
    mMainWndCaption = L"Picking";

    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    mCam.SetPosition(0.0f, 2.0f, -15.0f);

    XMMATRIX meshScale  = XMMatrixScaling(0.5f, 0.5f, 0.5f);
    XMMATRIX meshOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
    XMStoreFloat4x4(&mMeshWorld, XMMatrixMultiply(meshScale, meshOffset));

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

    mMeshMat.Ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
    mMeshMat.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    mMeshMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);

    mBoundingBoxMat.Ambient = XMFLOAT4(1.0f, 0.1f, 0.1f, 1.0f);
    mBoundingBoxMat.Diffuse = XMFLOAT4(1.0f, 0.1f, 0.1f, 1.0f);
    mBoundingBoxMat.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);

    mPickedTriangleMat.Ambient = XMFLOAT4(0.0f, 0.8f, 0.4f, 1.0f);
    mPickedTriangleMat.Diffuse = XMFLOAT4(0.0f, 0.8f, 0.4f, 1.0f);
    mPickedTriangleMat.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);
}

PickingApp::~PickingApp()
{
    ReleaseCOM(mMeshVB);
    ReleaseCOM(mMeshIB);

    Effects::DestroyAll();
    InputLayouts::DestroyAll();
    RenderStates::DestroyAll();
}

bool PickingApp::Init()
{
    if (!D3DApp::Init())
        return false;

    Effects::InitAll(md3dDevice);
    InputLayouts::InitAll(md3dDevice);
    RenderStates::InitAll(md3dDevice);

    BuildMeshGeometryBuffers();
    BuildBoundingBoxBuffer();

    return true;
}

void PickingApp::OnResize()
{
    D3DApp::OnResize();

    mCam.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
}

void PickingApp::UpdateScene(float dt)
{
    if (GetAsyncKeyState('W') & 0x8000)
        mCam.Walk(10.0f * dt);
    if (GetAsyncKeyState('S') & 0x8000)
        mCam.Walk(-10.0f * dt);
    if (GetAsyncKeyState('A') & 0x8000)
        mCam.Strafe(-10.0f * dt);
    if (GetAsyncKeyState('D') & 0x8000)
        mCam.Strafe(10.0f * dt);
}

void PickingApp::DrawScene()
{
    md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
    md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    UINT stride = sizeof(Vertex::Basic32);
    UINT offset = 0;

    mCam.UpdateViewMatrix();

    XMMATRIX view = mCam.View();
    XMMATRIX proj = mCam.Proj();
    
    Effects::BasicFX->SetDirLights(mDirLights);
    Effects::BasicFX->SetEyePosW(mCam.GetPosition());
    
    ID3DX11EffectTechnique* activeMeshTech = Effects::BasicFX->Light3Tech;

    D3DX11_TECHNIQUE_DESC techDesc;
    activeMeshTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        if (GetAsyncKeyState('1') & 0x8000)
            md3dImmediateContext->RSSetState(RenderStates::WireFrameRS);

        md3dImmediateContext->IASetVertexBuffers(0, 1, &mMeshVB, &stride, &offset);
        md3dImmediateContext->IASetIndexBuffer(mMeshIB, DXGI_FORMAT_R32_UINT, 0);

        XMMATRIX world = XMLoadFloat4x4(&mMeshWorld);
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
        XMMATRIX worldViewProj = world * view * proj;

        Effects::BasicFX->SetWorld(world);
        Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::BasicFX->SetWorldViewProj(worldViewProj);
        Effects::BasicFX->SetMaterial(mMeshMat);

        activeMeshTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexed(mMeshIndexCount, 0, 0);

        md3dImmediateContext->RSSetState(0);

        if (mPickedTriangle != -1)
        {
            md3dImmediateContext->OMSetDepthStencilState(RenderStates::LessEqualDSS, 0);

            Effects::BasicFX->SetMaterial(mPickedTriangleMat);
            activeMeshTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
            md3dImmediateContext->DrawIndexed(3, 3 * mPickedTriangle, 0);

            md3dImmediateContext->OMSetDepthStencilState(0, 0);
        }
    }

    activeMeshTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {

        md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoundingBoxVB, &stride, &offset);
        md3dImmediateContext->IASetIndexBuffer(mBoundingBoxIB, DXGI_FORMAT_R32_UINT, 0);

        md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

        XMMATRIX world = XMLoadFloat4x4(&mMeshWorld);
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
        XMMATRIX worldViewProj = world * view * proj;

        Effects::BasicFX->SetWorld(world);
        Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::BasicFX->SetWorldViewProj(worldViewProj);
        Effects::BasicFX->SetMaterial(mBoundingBoxMat);

        activeMeshTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexed(24, 0, 0);
    }

    HR(mSwapChain->Present(0, 0));
}


void PickingApp::OnMouseDown(WPARAM btnState, int x, int y)
{
    if ((btnState & MK_LBUTTON) != 0)
    {
        mLastMousePos.x = x;
        mLastMousePos.y = y;

        SetCapture(mhMainWnd);
    }
    else if ((btnState & MK_RBUTTON) != 0)
    {
        Pick(x, y);
    }
}

void PickingApp::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void PickingApp::OnMouseMove(WPARAM btnState, int x, int y)
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

void PickingApp::BuildMeshGeometryBuffers()
{
    std::ifstream fin("models/car.txt");

    if (!fin)
    {
        MessageBox(0, L"models/car.txt not found.", 0, 0);
        return;
    }

    UINT vCount = 0;
    UINT tCount = 0;
    std::string ignore;

    fin >> ignore >> vCount;
    fin >> ignore >> tCount;
    fin >> ignore >> ignore >> ignore >> ignore;

    XMFLOAT3 vMinf3(+MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity);
    XMFLOAT3 vMaxf3(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);

    XMVECTOR vMin = XMLoadFloat3(&vMinf3);
    XMVECTOR vMax = XMLoadFloat3(&vMaxf3);

    mMeshVertices.resize(vCount);
    for (UINT i = 0; i < vCount; ++i)
    {
        fin >> mMeshVertices[i].Pos.x    >> mMeshVertices[i].Pos.y    >> mMeshVertices[i].Pos.z;
        fin >> mMeshVertices[i].Normal.x >> mMeshVertices[i].Normal.y >> mMeshVertices[i].Normal.z;

        XMVECTOR P = XMLoadFloat3(&mMeshVertices[i].Pos);
        
        vMin = XMVectorMin(vMin, P);
        vMax = XMVectorMax(vMax, P);
    }

    mMeshBoxMin = vMin;
    mMeshBoxMax = vMax;

    XMStoreFloat3(&mMeshBox.Center,  0.5f * (vMin + vMax));
    XMStoreFloat3(&mMeshBox.Extents, 0.5f * (vMax - vMin));

    fin >> ignore >> ignore >> ignore;

    mMeshIndexCount = tCount * 3;
    mMeshIndices.resize(mMeshIndexCount);
    for (UINT i = 0; i < tCount; ++i)
    {
        fin >> mMeshIndices[i * 3 + 0] >> mMeshIndices[i * 3 + 1] >> mMeshIndices[i * 3 + 2];
    }

    fin.close();

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex::Basic32) * vCount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA vInitData;
    vInitData.pSysMem = &mMeshVertices[0];

    HR(md3dDevice->CreateBuffer(&vbd, &vInitData, &mMeshVB));

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * mMeshIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA iInitData;
    iInitData.pSysMem = &mMeshIndices[0];

    HR(md3dDevice->CreateBuffer(&ibd, &iInitData, &mMeshIB));
}

void PickingApp::BuildBoundingBoxBuffer()
{
    XMFLOAT3 boxMin, boxMax;
    XMStoreFloat3(&boxMin, mMeshBoxMin);
    XMStoreFloat3(&boxMax, mMeshBoxMax);

    std::vector<Vertex::Basic32> vertices(8);
    vertices[0].Pos = XMFLOAT3(boxMin.x, boxMin.y, boxMin.z);
    vertices[1].Pos = XMFLOAT3(boxMax.x, boxMin.y, boxMin.z);
    vertices[2].Pos = XMFLOAT3(boxMin.x, boxMax.y, boxMin.z);
    vertices[3].Pos = XMFLOAT3(boxMax.x, boxMax.y, boxMin.z);
    vertices[4].Pos = XMFLOAT3(boxMin.x, boxMin.y, boxMax.z);
    vertices[5].Pos = XMFLOAT3(boxMax.x, boxMin.y, boxMax.z);
    vertices[6].Pos = XMFLOAT3(boxMin.x, boxMax.y, boxMax.z);
    vertices[7].Pos = XMFLOAT3(boxMax.x, boxMax.y, boxMax.z);

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex::Basic32) * vertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA vInitData;
    vInitData.pSysMem = &vertices[0];

    HR(md3dDevice->CreateBuffer(&vbd, &vInitData, &mBoundingBoxVB));

    UINT indices[] = {
        // front face
        0, 1, 1, 3, 3, 2, 2, 0,
        // back face
        4, 5, 5, 7, 7, 6, 6, 4,
        // connecting edges
        0, 4, 1, 5, 2, 6, 3, 7
    };

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * 24;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = indices;

    HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mBoundingBoxIB));
}

void PickingApp::Pick(int sx, int sy)
{
    XMMATRIX P = mCam.Proj();

    // Compute picking ray in view space.
    float vx = (+2.0f * sx / mClientWidth - 1.0f)  / P.r[0].m128_f32[0];
    float vy = (-2.0f * sy / mClientHeight + 1.0f) / P.r[1].m128_f32[1];
    
    // Ray definition in view space.
    XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    XMVECTOR rayDir    = XMVectorSet(vx, vy, 1.0f, 0.0f);

    // Transform ray to local space of Mesh.
    XMMATRIX V = mCam.View();
    XMVECTOR detV = XMMatrixDeterminant(V);
    XMMATRIX invView = XMMatrixInverse(&detV, V);

    XMMATRIX W = XMLoadFloat4x4(&mMeshWorld);
    XMVECTOR detW = XMMatrixDeterminant(W);
    XMMATRIX invWorld = XMMatrixInverse(&detW, W);

    XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);

    rayOrigin = XMVector3TransformCoord(rayOrigin, toLocal);
    rayDir = XMVector3TransformNormal(rayDir, toLocal);

    // Make the ray direction unit length for the intersection tests.
    rayDir = XMVector3Normalize(rayDir);

    mPickedTriangle = -1;
    float tMin = 0.0f;
    if (Slaps(rayOrigin, rayDir, mMeshBoxMin, mMeshBoxMax, tMin))
    {
        OutputDebugStringA("The ray intersects the box.\n");
        tMin = MathHelper::Infinity;
        for (UINT i = 0; i < mMeshIndices.size() / 3; ++i)
        {
            // Indices for this triangle.
            UINT i0 = mMeshIndices[i * 3 + 0];
            UINT i1 = mMeshIndices[i * 3 + 1];
            UINT i2 = mMeshIndices[i * 3 + 2];

            // Vertices for this triangle.
            XMVECTOR v0 = XMLoadFloat3(&mMeshVertices[i0].Pos);
            XMVECTOR v1 = XMLoadFloat3(&mMeshVertices[i1].Pos);
            XMVECTOR v2 = XMLoadFloat3(&mMeshVertices[i2].Pos);

            // We have to iterate over all the triangles in order to find the nearest intersection.
            float t = 0.0f;
            if (TriangleTests::Intersects(rayOrigin, rayDir, v0, v1, v2, t))
            {
                if (t < tMin)
                {
                    // Thie is the new nearest picked triangle.
                    tMin = t;
                    mPickedTriangle = i;
                }
            }

        }
    }
    else
    {
        OutputDebugStringA("The ray does not intersect the box.\n");
    }
}

bool PickingApp::Slaps(DirectX::XMVECTOR rayOrigin,
                       DirectX::XMVECTOR rayDir,
                       DirectX::XMVECTOR boxMin,
                       DirectX::XMVECTOR boxMax,
                       float& tMin)
{
    float t1, t2;
    float stMin = -MathHelper::Infinity;
    float stMax = +MathHelper::Infinity;

    for (int i = 0; i < 3; ++i)
    {
        if (rayDir.m128_f32[i] != 0.0f)
        {
            t1 = (boxMin.m128_f32[i] - rayOrigin.m128_f32[i]) / rayDir.m128_f32[i];
            t2 = (boxMax.m128_f32[i] - rayOrigin.m128_f32[i]) / rayDir.m128_f32[i];

            if (t1 > t2)
                std::swap(t1, t2);

            stMin = MathHelper::Max(stMin, t1);
            stMax = MathHelper::Min(stMax, t2);
        }
        else
        {
            if (rayOrigin.m128_f32[i] < boxMin.m128_f32[i] || rayOrigin.m128_f32[i] > boxMax.m128_f32[i])
                return false;
        }
    }

    tMin = stMin;

    return stMax >= stMin && stMax >= 0.0f;
}
