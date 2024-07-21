#ifndef BLUR_FILTER_H
#define BLUR_FILTER_H

#include <Windows.h>
#include "D3DUtil.h"

class BlurFilter
{
public:
    BlurFilter();
    ~BlurFilter();

    ID3D11ShaderResourceView* GetBlurredOutput();
    
    // Generates Gaussian blur weights.
    void SetGaussianWeights(float sigma);

    // Manually sepcify blur weights.
    void SetWeights(const float weights[9]);

    void Init(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format);
    void BlurInPlace(ID3D11DeviceContext* context, ID3D11ShaderResourceView* inputSRV, ID3D11UnorderedAccessView* inputUAV, int blurCount);

private:
    UINT mWidth;
    UINT mHeight;
    DXGI_FORMAT mFormat;

    ID3D11ShaderResourceView* mBlurredOutputTexSRV;
    ID3D11UnorderedAccessView* mBlurredOutputTexUAV;
};

#endif /* BLUR_FILTER_H */