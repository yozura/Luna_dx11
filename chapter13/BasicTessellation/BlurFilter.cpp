#include "BlurFilter.h"
#include "Effects.h"

BlurFilter::BlurFilter()
    : mBlurredOutputTexSRV(0), mBlurredOutputTexUAV(0)
{
}

BlurFilter::~BlurFilter()
{
    ReleaseCOM(mBlurredOutputTexSRV);
    ReleaseCOM(mBlurredOutputTexUAV);
}

ID3D11ShaderResourceView* BlurFilter::GetBlurredOutput()
{
    return mBlurredOutputTexSRV;
}

void BlurFilter::SetGaussianWeights(float sigma)
{
    float d = 2.0f * sigma * sigma;
    
    float weights[9];
    float sum = 0.0f;

    for (int i = 0; i < 8; ++i)
    {
        float x = (float)i;
        weights[i] = expf(-x * x / d);
        sum += weights[i];
    }

    for (int i = 0; i < 8; ++i)
    {
        weights[i] /= sum;
    }

    Effects::BlurFX->SetWeights(weights);
}

void BlurFilter::SetWeights(const float weights[9])
{
    Effects::BlurFX->SetWeights(weights);
}

void BlurFilter::Init(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format)
{
    // Start Fresh.
    ReleaseCOM(mBlurredOutputTexSRV);
    ReleaseCOM(mBlurredOutputTexUAV);

    mWidth = width;
    mHeight = height;
    mFormat = format;

    D3D11_TEXTURE2D_DESC blurredTexDesc;
    blurredTexDesc.Width = width;
    blurredTexDesc.Height = height;
    blurredTexDesc.MipLevels = 1;
    blurredTexDesc.ArraySize = 1;
    blurredTexDesc.Format = format;
    blurredTexDesc.SampleDesc.Count = 1;
    blurredTexDesc.SampleDesc.Quality = 0;
    blurredTexDesc.Usage = D3D11_USAGE_DEFAULT;
    blurredTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    blurredTexDesc.CPUAccessFlags = 0;
    blurredTexDesc.MiscFlags = 0;

    ID3D11Texture2D* blurredTex = 0;
    HR(device->CreateTexture2D(&blurredTexDesc, 0, &blurredTex));

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    HR(device->CreateShaderResourceView(blurredTex, &srvDesc, &mBlurredOutputTexSRV));

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    uavDesc.Format = format;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice = 0;
    HR(device->CreateUnorderedAccessView(blurredTex, &uavDesc, &mBlurredOutputTexUAV));

    ReleaseCOM(blurredTex);
}

void BlurFilter::BlurInPlace(ID3D11DeviceContext* context, ID3D11ShaderResourceView* inputSRV, ID3D11UnorderedAccessView* inputUAV, int blurCount)
{
    for (int i = 0; i < blurCount; ++i)
    {
        D3DX11_TECHNIQUE_DESC techDesc;
        Effects::BlurFX->HorzBlurTech->GetDesc(&techDesc);
        for (UINT p = 0; p < techDesc.Passes; ++p)
        {
            Effects::BlurFX->SetInputMap(inputSRV);
            Effects::BlurFX->SetOutputMap(mBlurredOutputTexUAV);
            Effects::BlurFX->HorzBlurTech->GetPassByIndex(p)->Apply(0, context);
            
            UINT numGroupsX = (UINT)ceilf(mWidth / 256.0f);
            context->Dispatch(numGroupsX, mHeight, 1);
        }

        ID3D11ShaderResourceView* nullSRV[1] = { 0 };
        context->CSSetShaderResources(0, 1, nullSRV);

        ID3D11UnorderedAccessView* nullUAV[1] = { 0 };
        context->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

        Effects::BlurFX->VertBlurTech->GetDesc(&techDesc);
        for (UINT p = 0; p < techDesc.Passes; ++p)
        {
            Effects::BlurFX->SetInputMap(mBlurredOutputTexSRV);
            Effects::BlurFX->SetOutputMap(inputUAV);
            Effects::BlurFX->VertBlurTech->GetPassByIndex(p)->Apply(0, context);

            UINT numGropusY = (UINT)ceilf(mHeight / 256.0f);
            context->Dispatch(mWidth, numGropusY, 1);
        }
        
        context->CSSetShaderResources(0, 1, nullSRV);
        context->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);
    }

    context->CSSetShader(0, 0, 0);
}
