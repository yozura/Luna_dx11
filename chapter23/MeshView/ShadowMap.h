#ifndef SHADOW_MAP_H
#define SHADOW_MAP_H

#include "D3DUtil.h"
#include "Camera.h"

class ShadowMap
{
public:
    ShadowMap(ID3D11Device* device, UINT width, UINT length);
    ~ShadowMap();

    ID3D11ShaderResourceView* DepthMapSRV();

    void BindDSVAndSetNullRenderTarget(ID3D11DeviceContext* dc);

private:
    ShadowMap(const ShadowMap& rhs);
    ShadowMap& operator=(const ShadowMap& rhs);

private:
    UINT mWidth;
    UINT mHeight;

    ID3D11ShaderResourceView* mDepthMapSRV;
    ID3D11DepthStencilView* mDepthMapDSV;

    D3D11_VIEWPORT mViewport;
};

#endif /* SHADOW_MAP_H */
