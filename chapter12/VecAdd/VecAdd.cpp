#include "VecAdd.h"

using namespace DirectX;

VecAdd::VecAdd(HINSTANCE hInstance)
    : D3DApp(hInstance), mNumElements(32)
    , mOutputBuffer(0), mOutputDebugBuffer(0)
    , mInputASRV(0), mInputBSRV(0), mOutputUAV(0)
{
    mMainWndCaption = L"Compute Shader VecAdd";
}

VecAdd::~VecAdd()
{
    md3dImmediateContext->ClearState();

    ReleaseCOM(mOutputBuffer);
    ReleaseCOM(mOutputDebugBuffer);
    
    ReleaseCOM(mInputASRV);
    ReleaseCOM(mInputBSRV);
    ReleaseCOM(mOutputUAV);

    Effects::DestroyAll();
    InputLayouts::DestroyAll();
    RenderStates::DestroyAll();
}

bool VecAdd::Init()
{
    if (!D3DApp::Init())
        return false;

    Effects::InitAll(md3dDevice);
    InputLayouts::InitAll(md3dDevice);
    RenderStates::InitAll(md3dDevice);

    BuildBuffersAndView();

    return true;
}

void VecAdd::OnResize()
{
    D3DApp::OnResize();
}

void VecAdd::UpdateScene(float dt)
{
}

void VecAdd::DrawScene()
{
    md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
    md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    HR(mSwapChain->Present(0, 0));
}

void VecAdd::DoComputeWork()
{
    D3DX11_TECHNIQUE_DESC techDesc;

    Effects::VecAddFX->SetInputA(mInputASRV);
    Effects::VecAddFX->SetInputB(mInputBSRV);
    Effects::VecAddFX->SetOutput(mOutputUAV);

    Effects::VecAddFX->VecAddTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        ID3DX11EffectPass* pass = Effects::VecAddFX->VecAddTech->GetPassByIndex(p);
        pass->Apply(0, md3dImmediateContext);

        md3dImmediateContext->Dispatch(1, 1, 1);
    }

    ID3D11ShaderResourceView* nullSRV[1] = { 0 };
    md3dImmediateContext->CSSetShaderResources(0, 1, nullSRV);

    ID3D11UnorderedAccessView* nullUAV[1] = { 0 };
    md3dImmediateContext->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

    md3dImmediateContext->CSSetShader(0, 0, 0);

    std::ofstream fout("results.txt");

    md3dImmediateContext->CopyResource(mOutputDebugBuffer, mOutputBuffer);

    D3D11_MAPPED_SUBRESOURCE mappedData;
    md3dImmediateContext->Map(mOutputDebugBuffer, 0, D3D11_MAP_READ, 0, &mappedData);

    Data* dataView = reinterpret_cast<Data*>(mappedData.pData);
    for (int i = 0; i < (int)mNumElements; ++i)
    {
        fout << "("  << dataView[i].v1.x << ", " << dataView[i].v1.y << ", " << dataView[i].v1.z
             << ", " << dataView[i].v2.x << ", " << dataView[i].v2.y << ")"  << std::endl;
    }

    md3dImmediateContext->Unmap(mOutputDebugBuffer, 0);

    fout.close();
}

void VecAdd::BuildBuffersAndView()
{
    std::vector<Data> dataA(mNumElements);
    std::vector<Data> dataB(mNumElements);
    for (int i = 0; i < (int)mNumElements; ++i)
    {
        dataA[i].v1 = XMFLOAT3(i, i, i);
        dataA[i].v2 = XMFLOAT2(i, 0);

        dataB[i].v1 = XMFLOAT3(-i, i, 0.0f);
        dataB[i].v2 = XMFLOAT2(0, -i);
    }

    D3D11_BUFFER_DESC inputDesc;
    inputDesc.Usage = D3D11_USAGE_DEFAULT;
    inputDesc.ByteWidth = sizeof(Data) * mNumElements;
    inputDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    inputDesc.CPUAccessFlags = 0;
    inputDesc.StructureByteStride = sizeof(Data);
    inputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    D3D11_SUBRESOURCE_DATA vInitDataA;
    vInitDataA.pSysMem = &dataA[0];

    ID3D11Buffer* bufferA = 0;
    HR(md3dDevice->CreateBuffer(&inputDesc, &vInitDataA, &bufferA));

    D3D11_SUBRESOURCE_DATA vInitDataB;
    vInitDataB.pSysMem = &dataB[0];

    ID3D11Buffer* bufferB = 0;
    HR(md3dDevice->CreateBuffer(&inputDesc, &vInitDataB, &bufferB));

    D3D11_BUFFER_DESC outputDesc;
    outputDesc.Usage = D3D11_USAGE_DEFAULT;
    outputDesc.ByteWidth = sizeof(Data) * mNumElements;
    outputDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
    outputDesc.CPUAccessFlags = 0;
    outputDesc.StructureByteStride = sizeof(Data);
    outputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    HR(md3dDevice->CreateBuffer(&outputDesc, 0, &mOutputBuffer));

    outputDesc.Usage = D3D11_USAGE_STAGING;
    outputDesc.BindFlags = 0;
    outputDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    HR(md3dDevice->CreateBuffer(&outputDesc, 0, &mOutputDebugBuffer));

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
    srvDesc.BufferEx.FirstElement = 0;
    srvDesc.BufferEx.Flags = 0;
    srvDesc.BufferEx.NumElements = mNumElements;

    HR(md3dDevice->CreateShaderResourceView(bufferA, &srvDesc, &mInputASRV));
    HR(md3dDevice->CreateShaderResourceView(bufferB, &srvDesc, &mInputBSRV));
    
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.Flags = 0;
    uavDesc.Buffer.NumElements = mNumElements;
    
    HR(md3dDevice->CreateUnorderedAccessView(mOutputBuffer, &uavDesc, &mOutputUAV));

    ReleaseCOM(bufferA);
    ReleaseCOM(bufferB);
}