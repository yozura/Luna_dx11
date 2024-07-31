#include "NormalDisplacementMap.h"

using namespace DirectX;

NormalDisplacementMap::NormalDisplacementMap(HINSTANCE hInstance)
    : D3DApp(hInstance)
    , mSky(0)
    , mShapesVB(0), mShapesIB(0)
    , mSkullVB(0), mSkullIB(0)
    , mSkullIndexCount(0), mStoneTexSRV(0)
    , mBrickTexSRV(0), mLightCount(3)
    , mStoneNormalTexSRV(0), mBrickNormalTexSRV(0)
    , mRenderOptions(RenderOptions::RenderOptionsNormalMap)
{
    mMainWndCaption = L"NormalDisplacementMap";

    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    mCam.SetPosition(0.0f, 2.0f, -15.0f);

    XMMATRIX I = XMMatrixIdentity();
    XMStoreFloat4x4(&mGridWorld, I);

    XMMATRIX boxScale  = XMMatrixScaling(3.0f, 1.0f, 3.0f);
    XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
    XMStoreFloat4x4(&mBoxWorld, XMMatrixMultiply(boxScale, boxOffset));

    XMMATRIX skullScale  = XMMatrixScaling(0.5f, 0.5f, 0.5f);
    XMMATRIX skullOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
    XMStoreFloat4x4(&mSkullWorld, XMMatrixMultiply(skullScale, skullOffset));

    for (int i = 0; i < 5; ++i)
    {
        XMStoreFloat4x4(&mCylinderWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f));
        XMStoreFloat4x4(&mCylinderWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f));

        XMStoreFloat4x4(&mSphereWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f));
        XMStoreFloat4x4(&mSphereWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f));
    }

    mDirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirLights[0].Diffuse = XMFLOAT4(0.7f, 0.7f, 0.6f, 1.0f);
    mDirLights[0].Specular = XMFLOAT4(0.8f, 0.8f, 0.7f, 1.0f);
    mDirLights[0].Direction = XMFLOAT3(0.707f, 0.0f, 0.707f);

    mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mDirLights[1].Diffuse = XMFLOAT4(0.40f, 0.40f, 0.40f, 1.0f);
    mDirLights[1].Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirLights[1].Direction = XMFLOAT3(0.0f, -0.707f, 0.707f);

    mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mDirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirLights[2].Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirLights[2].Direction = XMFLOAT3(-0.57735f, -0.57735f, -0.57735f);

    mGridMat.Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    mGridMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mGridMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
    mGridMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

    mCylinderMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mCylinderMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mCylinderMat.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 32.0f);
    mCylinderMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

    mSphereMat.Ambient = XMFLOAT4(0.2f, 0.3f, 0.4f, 1.0f);
    mSphereMat.Diffuse = XMFLOAT4(0.2f, 0.3f, 0.4f, 1.0f);
    mSphereMat.Specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f);
    mSphereMat.Reflect = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);

    mBoxMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mBoxMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mBoxMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
    mBoxMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

    mSkullMat.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mSkullMat.Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mSkullMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
    mSkullMat.Reflect = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
}

NormalDisplacementMap::~NormalDisplacementMap()
{
    SafeDelete(mSky);

    ReleaseCOM(mShapesVB);
    ReleaseCOM(mShapesIB);
    ReleaseCOM(mSkullVB);
    ReleaseCOM(mSkullIB);

    ReleaseCOM(mStoneTexSRV);
    ReleaseCOM(mBrickTexSRV);
    
    ReleaseCOM(mStoneNormalTexSRV);
    ReleaseCOM(mBrickNormalTexSRV);

    Effects::DestroyAll();
    InputLayouts::DestroyAll();
    RenderStates::DestroyAll();
}

bool NormalDisplacementMap::Init()
{
    if (!D3DApp::Init())
        return false;

    Effects::InitAll(md3dDevice);
    InputLayouts::InitAll(md3dDevice);
    RenderStates::InitAll(md3dDevice);

    mSky = new Sky(md3dDevice, L"textures/snowcube1024.dds", 5000.0f);

    ScratchImage floor;
    HR(LoadFromDDSFile(L"textures/stones.dds", DDS_FLAGS_NONE, nullptr, floor));
    HR(CreateShaderResourceView(md3dDevice, floor.GetImages(), floor.GetImageCount(), floor.GetMetadata(), &mStoneTexSRV));

    ScratchImage bricks;
    HR(LoadFromDDSFile(L"textures/bricks.dds", DDS_FLAGS_NONE, nullptr, bricks));
    HR(CreateShaderResourceView(md3dDevice, bricks.GetImages(), bricks.GetImageCount(), bricks.GetMetadata(), &mBrickTexSRV));

    ScratchImage floorNormal;
    HR(LoadFromDDSFile(L"textures/stones_nmap.dds", DDS_FLAGS_NONE, nullptr, floorNormal));
    HR(CreateShaderResourceView(md3dDevice, floorNormal.GetImages(), floorNormal.GetImageCount(), floorNormal.GetMetadata(), &mStoneNormalTexSRV));

    ScratchImage bricksNormal;
    HR(LoadFromDDSFile(L"textures/bricks_nmap.dds", DDS_FLAGS_NONE, nullptr, bricksNormal));
    HR(CreateShaderResourceView(md3dDevice, bricksNormal.GetImages(), bricksNormal.GetImageCount(), bricksNormal.GetMetadata(), &mBrickNormalTexSRV));

    BuildShapeGeometryBuffers();
    BuildSkullGeometryBuffers();

    return true;
}

void NormalDisplacementMap::OnResize()
{
    D3DApp::OnResize();

    mCam.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
}

void NormalDisplacementMap::UpdateScene(float dt)
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
        mRenderOptions = RenderOptionsBasic;
    if (GetAsyncKeyState('2') & 0x8000)
        mRenderOptions = RenderOptionsNormalMap;
    if (GetAsyncKeyState('3') & 0x8000)
        mRenderOptions = RenderOptionsDisplacementMap;
}

void NormalDisplacementMap::DrawScene()
{
    md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
    md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    mCam.UpdateViewMatrix();

    XMMATRIX view = mCam.View();
    XMMATRIX proj = mCam.Proj();
    XMMATRIX viewProj = mCam.ViewProj();
    
    float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    
    Effects::BasicFX->SetDirLights(mDirLights);
    Effects::BasicFX->SetEyePosW(mCam.GetPosition());
    Effects::BasicFX->SetCubeMap(mSky->CubeMapSRV());

    Effects::NormalMapFX->SetDirLights(mDirLights);
    Effects::NormalMapFX->SetEyePosW(mCam.GetPosition());
    Effects::NormalMapFX->SetCubeMap(mSky->CubeMapSRV());

    Effects::DisplacementMapFX->SetDirLights(mDirLights);
    Effects::DisplacementMapFX->SetEyePosW(mCam.GetPosition());
    Effects::DisplacementMapFX->SetCubeMap(mSky->CubeMapSRV());

    Effects::DisplacementMapFX->SetHeightScale(0.07f);
    Effects::DisplacementMapFX->SetMaxTessDistance(1.0f);
    Effects::DisplacementMapFX->SetMinTessDistance(25.0f);
    Effects::DisplacementMapFX->SetMinTessFactor(1.0f);
    Effects::DisplacementMapFX->SetMaxTessFactor(5.0f);

    ID3DX11EffectTechnique* activeTech       = Effects::DisplacementMapFX->Light3TexTech;
    ID3DX11EffectTechnique* activeSphereTech = Effects::BasicFX->Light3ReflectTech;
    ID3DX11EffectTechnique* activeSkullTech  = Effects::BasicFX->Light3ReflectTech;
    switch (mRenderOptions)
    {
    case RenderOptionsBasic:
        activeTech = Effects::BasicFX->Light3TexTech;
        md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        break;
    case RenderOptionsNormalMap:
        activeTech = Effects::NormalMapFX->Light3TexTech;
        md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        break;
    case RenderOptionsDisplacementMap:
        activeTech = Effects::DisplacementMapFX->Light3TexTech;
        md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
        break;
    }

    XMMATRIX world;
    XMMATRIX worldInvTranspose;
    XMMATRIX worldViewProj;

    UINT stride = sizeof(Vertex::PosNormalTexTan);
    UINT offset = 0;

    md3dImmediateContext->IASetInputLayout(InputLayouts::PosNormalTexTan);
    md3dImmediateContext->IASetVertexBuffers(0, 1, &mShapesVB, &stride, &offset);
    md3dImmediateContext->IASetIndexBuffer(mShapesIB, DXGI_FORMAT_R32_UINT, 0);

    if (GetAsyncKeyState('R') & 0x8000)
        md3dImmediateContext->RSSetState(RenderStates::WireFrameRS);

    D3DX11_TECHNIQUE_DESC techDesc;
    activeTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        // Grid
        world = XMLoadFloat4x4(&mGridWorld);
        worldInvTranspose = MathHelper::InverseTranspose(world);
        worldViewProj = world * view * proj;

        switch (mRenderOptions)
        {
        case RenderOptionsBasic:
            Effects::BasicFX->SetWorld(world);
            Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
            Effects::BasicFX->SetWorldViewProj(worldViewProj);
            Effects::BasicFX->SetTexTransform(XMMatrixScaling(8.0f, 10.0f, 1.0f));
            Effects::BasicFX->SetMaterial(mGridMat);
            Effects::BasicFX->SetDiffuseMap(mStoneTexSRV);
            break;
        case RenderOptionsNormalMap:
            Effects::NormalMapFX->SetWorld(world);
            Effects::NormalMapFX->SetWorldInvTranspose(worldInvTranspose);
            Effects::NormalMapFX->SetWorldViewProj(worldViewProj);
            Effects::NormalMapFX->SetTexTransform(XMMatrixScaling(8.0f, 10.0f, 1.0f));
            Effects::NormalMapFX->SetMaterial(mGridMat);
            Effects::NormalMapFX->SetDiffuseMap(mStoneTexSRV);
            Effects::NormalMapFX->SetNormalMap(mStoneNormalTexSRV);
            break;
        case RenderOptionsDisplacementMap:
            Effects::DisplacementMapFX->SetWorld(world);
            Effects::DisplacementMapFX->SetWorldInvTranspose(worldInvTranspose);
            Effects::DisplacementMapFX->SetViewProj(viewProj);
            Effects::DisplacementMapFX->SetWorldViewProj(worldViewProj);
            Effects::DisplacementMapFX->SetTexTransform(XMMatrixScaling(8.0f, 10.0f, 1.0f));
            Effects::DisplacementMapFX->SetMaterial(mGridMat);
            Effects::DisplacementMapFX->SetDiffuseMap(mStoneTexSRV);
            Effects::DisplacementMapFX->SetNormalMap(mStoneNormalTexSRV);
            break;
        }

        activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

        // Box
        world = XMLoadFloat4x4(&mBoxWorld);
        worldInvTranspose = MathHelper::InverseTranspose(world);
        worldViewProj = world * view * proj;

        switch (mRenderOptions)
        {
        case RenderOptionsBasic:
            Effects::BasicFX->SetWorld(world);
            Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
            Effects::BasicFX->SetWorldViewProj(worldViewProj);
            Effects::BasicFX->SetTexTransform(XMMatrixScaling(2.0f, 1.0f, 1.0f));
            Effects::BasicFX->SetMaterial(mBoxMat);
            Effects::BasicFX->SetDiffuseMap(mBrickTexSRV);
            break;
        case RenderOptionsNormalMap:
            Effects::NormalMapFX->SetWorld(world);
            Effects::NormalMapFX->SetWorldInvTranspose(worldInvTranspose);
            Effects::NormalMapFX->SetWorldViewProj(worldViewProj);
            Effects::NormalMapFX->SetTexTransform(XMMatrixScaling(2.0f, 1.0f, 1.0f));
            Effects::NormalMapFX->SetMaterial(mBoxMat);
            Effects::NormalMapFX->SetDiffuseMap(mBrickTexSRV);
            Effects::NormalMapFX->SetNormalMap(mBrickNormalTexSRV);
            break;
        case RenderOptionsDisplacementMap:
            Effects::DisplacementMapFX->SetWorld(world);
            Effects::DisplacementMapFX->SetWorldInvTranspose(worldInvTranspose);
            Effects::DisplacementMapFX->SetViewProj(viewProj);
            Effects::DisplacementMapFX->SetWorldViewProj(worldViewProj);
            Effects::DisplacementMapFX->SetTexTransform(XMMatrixScaling(2.0f, 1.0f, 1.0f));
            Effects::DisplacementMapFX->SetMaterial(mBoxMat);
            Effects::DisplacementMapFX->SetDiffuseMap(mBrickTexSRV);
            Effects::DisplacementMapFX->SetNormalMap(mBrickNormalTexSRV);
            break;
        }

        activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

        // Cylinders
        for (UINT i = 0; i < 10; ++i)
        {
            world = XMLoadFloat4x4(&mCylinderWorld[i]);
            worldInvTranspose = MathHelper::InverseTranspose(world);
            worldViewProj = world * view * proj;

            switch (mRenderOptions)
            {
            case RenderOptionsBasic:
                Effects::BasicFX->SetWorld(world);
                Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
                Effects::BasicFX->SetWorldViewProj(worldViewProj);
                Effects::BasicFX->SetTexTransform(XMMatrixScaling(1.0f, 2.0f, 1.0f));
                Effects::BasicFX->SetMaterial(mCylinderMat);
                Effects::BasicFX->SetDiffuseMap(mBrickTexSRV);
                break;
            case RenderOptionsNormalMap:
                Effects::NormalMapFX->SetWorld(world);
                Effects::NormalMapFX->SetWorldInvTranspose(worldInvTranspose);
                Effects::NormalMapFX->SetWorldViewProj(worldViewProj);
                Effects::NormalMapFX->SetTexTransform(XMMatrixScaling(1.0f, 2.0f, 1.0f));
                Effects::NormalMapFX->SetMaterial(mCylinderMat);
                Effects::NormalMapFX->SetDiffuseMap(mBrickTexSRV);
                Effects::NormalMapFX->SetNormalMap(mBrickNormalTexSRV);
                break;
            case RenderOptionsDisplacementMap:
                Effects::DisplacementMapFX->SetWorld(world);
                Effects::DisplacementMapFX->SetWorldInvTranspose(worldInvTranspose);
                Effects::DisplacementMapFX->SetViewProj(viewProj);
                Effects::DisplacementMapFX->SetWorldViewProj(worldViewProj);
                Effects::DisplacementMapFX->SetTexTransform(XMMatrixScaling(1.0f, 2.0f, 1.0f));
                Effects::DisplacementMapFX->SetMaterial(mCylinderMat);
                Effects::DisplacementMapFX->SetDiffuseMap(mBrickTexSRV);
                Effects::DisplacementMapFX->SetNormalMap(mBrickNormalTexSRV);
                break;
            }

            activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
            md3dImmediateContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
        }
    }

    md3dImmediateContext->HSSetShader(0, 0, 0);
    md3dImmediateContext->DSSetShader(0, 0, 0);

    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    activeSphereTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        // Sphere
        for (UINT i = 0; i < 10; ++i)
        {
            world = XMLoadFloat4x4(&mSphereWorld[i]);
            worldInvTranspose = MathHelper::InverseTranspose(world);
            worldViewProj = world * view * proj;

            Effects::BasicFX->SetWorld(world);
            Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
            Effects::BasicFX->SetWorldViewProj(worldViewProj);
            Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
            Effects::BasicFX->SetMaterial(mSphereMat);

            activeSphereTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
            md3dImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
        }
    }

    stride = sizeof(Vertex::Basic32);
    offset = 0;

    md3dImmediateContext->RSSetState(0);
    
    md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
    md3dImmediateContext->IASetVertexBuffers(0, 1, &mSkullVB, &stride, &offset);
    md3dImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);
    
    activeSkullTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        // Skull
        world = XMLoadFloat4x4(&mSkullWorld);
        worldInvTranspose = MathHelper::InverseTranspose(world);
        worldViewProj = world * view * proj;

        Effects::BasicFX->SetWorld(world);
        Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::BasicFX->SetWorldViewProj(worldViewProj);
        Effects::BasicFX->SetMaterial(mSkullMat);

        activeSkullTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);
    }

    mSky->Draw(md3dImmediateContext, mCam);

    // restore default states. as the SkyFX changes them in the effect file.
    md3dImmediateContext->RSSetState(0);
    md3dImmediateContext->OMSetDepthStencilState(0, 0);

    HR(mSwapChain->Present(0, 0));
}


void NormalDisplacementMap::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void NormalDisplacementMap::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void NormalDisplacementMap::OnMouseMove(WPARAM btnState, int x, int y)
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

void NormalDisplacementMap::BuildShapeGeometryBuffers()
{
    GeometryGenerator::MeshData box;
    GeometryGenerator::MeshData grid;
    GeometryGenerator::MeshData sphere;
    GeometryGenerator::MeshData cylinder;

    GeometryGenerator geoGen;
    geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
    geoGen.CreateGrid(20.0f, 30.0f, 60, 40, grid);
    geoGen.CreateSphere(0.5f, 20, 20, sphere);
    geoGen.CreateCylinder(0.5f, 0.5f, 3.0f, 15, 15, cylinder);

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

    std::vector<Vertex::PosNormalTexTan> vertices(totalVertexCount);
    UINT k = 0;
    for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos      = box.Vertices[i].Position;
        vertices[k].Normal   = box.Vertices[i].Normal;
        vertices[k].Tex      = box.Vertices[i].TexC;
        vertices[k].TangentU = box.Vertices[i].TangentU;
    }

    for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos      = grid.Vertices[i].Position;
        vertices[k].Normal   = grid.Vertices[i].Normal;
        vertices[k].Tex      = grid.Vertices[i].TexC;
        vertices[k].TangentU = grid.Vertices[i].TangentU;
    }
    
    for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos      = sphere.Vertices[i].Position;
        vertices[k].Normal   = sphere.Vertices[i].Normal;
        vertices[k].Tex      = sphere.Vertices[i].TexC;
        vertices[k].TangentU = sphere.Vertices[i].TangentU;
    }

    for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos      = cylinder.Vertices[i].Position;
        vertices[k].Normal   = cylinder.Vertices[i].Normal;
        vertices[k].Tex      = cylinder.Vertices[i].TexC;
        vertices[k].TangentU = cylinder.Vertices[i].TangentU;
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex::PosNormalTexTan) * totalVertexCount;
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

void NormalDisplacementMap::BuildSkullGeometryBuffers()
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