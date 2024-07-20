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

    static ID3D11BlendState* AlphaToCoverageBS;
    static ID3D11BlendState* TransparentBS;
    static ID3D11BlendState* DiscardRedGreenBS;
};

#endif /* RENDER STATES_H */