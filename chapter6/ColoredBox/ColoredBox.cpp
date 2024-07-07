#include "ColoredBox.h"

ColoredBox::ColoredBox(HINSTANCE hInstance)
    : D3DApp(hInstance),
    mCBPosVertexBuffer(0),
    mCBColorVertexBuffer(0),
    mCBIndexBuffer(0),
    mFX(0),
    mTech(0),
    mfxWorldViewProj(0),
    mInputLayout(0),
    mTheta(1.5f * MathHelper::Pi),
    mPhi(0.25f * MathHelper::Pi),
    mRadius(5.0f)
{
    mMainWndCaption = L"Box";

    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    XMMATRIX I = XMMatrixIdentity();
    XMStoreFloat4x4(&mWorld, I);
    XMStoreFloat4x4(&mView, I);
    XMStoreFloat4x4(&mProj, I);
}

ColoredBox::~ColoredBox()
{
    ReleaseCOM(mCBPosVertexBuffer);
    ReleaseCOM(mCBColorVertexBuffer);
    ReleaseCOM(mCBIndexBuffer);
    ReleaseCOM(mFX);
    ReleaseCOM(mInputLayout);
}

bool ColoredBox::Init()
{
    if (!D3DApp::Init())
        return false;

    BuildGeometryBuffers();
    BuildFX();
    BuildVertexLayout();

    return true;
}

void ColoredBox::OnResize()
{
    D3DApp::OnResize();

    // â ũ�Ⱑ �缳�� �Ǿ����Ƿ� ���� ���� ����� ��Ⱦ�� �°� �缳��.
    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj, P);
}

void ColoredBox::UpdateScene(float dt)
{
    // ���� ��ǥ�� ���� ��ǥ�� ��ȯ
    float x = mRadius * sinf(mPhi) * cosf(mTheta);
    float z = mRadius * sinf(mPhi) * sinf(mTheta);
    float y = mRadius * cosf(mPhi);

    // �� ��� ����
    XMVECTOR pos    = XMVectorSet(x, y, z, 1.0f);
    XMVECTOR target = XMVectorZero();
    XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
    XMStoreFloat4x4(&mView, V);
}

void ColoredBox::DrawScene()
{
    md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
    md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // �Է� �����
    md3dImmediateContext->IASetInputLayout(mInputLayout);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // ���� & �ε��� ���� ����
    UINT stride[] = { sizeof(XMFLOAT3), sizeof(XMFLOAT4) };
    UINT offset[] = { 0, 0 };

    ID3D11Buffer* vertexBuffer[] = { mCBPosVertexBuffer, mCBColorVertexBuffer };
    md3dImmediateContext->IASetVertexBuffers(0, 2, vertexBuffer, stride, offset);
    md3dImmediateContext->IASetIndexBuffer(mCBIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // ��� ���� ����
    XMMATRIX world = XMLoadFloat4x4(&mWorld);
    XMMATRIX view  = XMLoadFloat4x4(&mView);
    XMMATRIX proj  = XMLoadFloat4x4(&mProj);
    XMMATRIX worldViewProj = world * view * proj;

    mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
    
    D3DX11_TECHNIQUE_DESC techDesc;
    mTech->GetDesc(&techDesc);

    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexed(36, 0, 0);
    }

    HR(mSwapChain->Present(0, 0));
}

void ColoredBox::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void ColoredBox::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void ColoredBox::OnMouseMove(WPARAM btnState, int x, int y)
{
    if ((btnState & MK_LBUTTON) != 0)
    {
        float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
        float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

        mTheta += dx;
        mPhi   += dy;

        mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
    }
    else if ((btnState & MK_RBUTTON) != 0)
    {
        float dx = 0.005f * static_cast<float>(x - mLastMousePos.x);
        float dy = 0.005f * static_cast<float>(y - mLastMousePos.y);

        mRadius += dx - dy;

        mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}

void ColoredBox::BuildGeometryBuffers()
{
    // ���� ����
    XMFLOAT3 verticesPos[] =
    {
        XMFLOAT3(-1.0f, -1.0f, -1.0f),
        XMFLOAT3(-1.0f, +1.0f, -1.0f),
        XMFLOAT3(+1.0f, +1.0f, -1.0f),
        XMFLOAT3(+1.0f, -1.0f, -1.0f),
        XMFLOAT3(-1.0f, -1.0f, +1.0f),
        XMFLOAT3(-1.0f, +1.0f, +1.0f),
        XMFLOAT3(+1.0f, +1.0f, +1.0f),
        XMFLOAT3(+1.0f, -1.0f, +1.0f),
    };

    XMFLOAT4 verticesColor[] =
    {
        XMFLOAT4(Colors::White),
        XMFLOAT4(Colors::Black),
        XMFLOAT4(Colors::Red),
        XMFLOAT4(Colors::Green),
        XMFLOAT4(Colors::Blue),
        XMFLOAT4(Colors::Yellow),
        XMFLOAT4(Colors::Cyan),
        XMFLOAT4(Colors::Magenta)
    };

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(XMFLOAT3) * 8;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    vbd.StructureByteStride = 0;
    
    D3D11_SUBRESOURCE_DATA vpInitData;
    vpInitData.pSysMem = verticesPos;
    
    HR(md3dDevice->CreateBuffer(&vbd, &vpInitData, &mCBPosVertexBuffer));

    D3D11_BUFFER_DESC vcbd;
    vcbd.Usage = D3D11_USAGE_IMMUTABLE;
    vcbd.ByteWidth = sizeof(XMFLOAT4) * 8;
    vcbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vcbd.CPUAccessFlags = 0;
    vcbd.MiscFlags = 0;
    vcbd.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA vcInitData;
    vcInitData.pSysMem = verticesColor;

    HR(md3dDevice->CreateBuffer(&vcbd, &vcInitData, &mCBColorVertexBuffer));

    // �ε��� ����
    UINT indices[] =
    {
        // ��
        0, 1, 2,
        0, 2, 3,

        // ��
        4, 6, 5,
        4, 7, 6,

        // ����
        4, 5, 1,
        4, 1, 0,

        // ������
        3, 2, 6,
        3, 6, 7,

        // ��
        1, 5, 6,
        1, 6, 2,

        // �Ʒ�
        4, 0, 3,
        4, 3, 7
    };

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * 36;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    ibd.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA iInitData;
    iInitData.pSysMem = indices;
    
    HR(md3dDevice->CreateBuffer(&ibd, &iInitData, &mCBIndexBuffer));
}

void ColoredBox::BuildFX()
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

void ColoredBox::BuildVertexLayout()
{
    D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    D3DX11_PASS_DESC passDesc;
    mTech->GetPassByIndex(0)->GetDesc(&passDesc);
    HR(md3dDevice->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mInputLayout));
}