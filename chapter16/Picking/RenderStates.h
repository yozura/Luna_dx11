#ifndef RENDER_STATES_H
#define RENDER_STATES_H

#include "D3DUtil.h"

class RenderStates
{
public:
    static void InitAll(ID3D11Device* device);
    static void DestroyAll();

    static ID3D11RasterizerState* WireFrameRS;
    static ID3D11RasterizerState* NoCullRS;
    static ID3D11RasterizerState* CullClockwiseRS;

    static ID3D11BlendState* AlphaToCoverageBS;
    static ID3D11BlendState* TransparentBS;
    static ID3D11BlendState* DiscardRedGreenBS;
    static ID3D11BlendState* NoRenderTargetWriteBS;

    static ID3D11DepthStencilState* MarkMirrorDSS;
    static ID3D11DepthStencilState* DrawReflectDSS;
    static ID3D11DepthStencilState* NoDoubleBlendDSS;
    static ID3D11DepthStencilState* LessEqualDSS;
};

#endif /* RENDER STATES_H */