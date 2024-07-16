#include "RenderStates.h"

ID3D11RasterizerState* RenderStates::WireFrameRS  = 0;
ID3D11RasterizerState* RenderStates::NoCullRS     = 0;

ID3D11BlendState* RenderStates::AlphaToCoverageBS = 0;
ID3D11BlendState* RenderStates::TransparentBS     = 0;

void RenderStates::InitAll(ID3D11Device* device)
{
    D3D11_RASTERIZER_DESC rd;
    D3D11_BLEND_DESC      bd;

    // WireFrameRS
    ZeroMemory(&rd, sizeof(D3D11_RASTERIZER_DESC));
    rd.FillMode = D3D11_FILL_WIREFRAME;
    rd.CullMode = D3D11_CULL_BACK;
    rd.FrontCounterClockwise = false;
    rd.DepthClipEnable = true;
    HR(device->CreateRasterizerState(&rd, &WireFrameRS));

    // NoCullRS
    ZeroMemory(&rd, sizeof(D3D11_RASTERIZER_DESC));
    rd.FillMode = D3D11_FILL_SOLID;
    rd.CullMode = D3D11_CULL_NONE;
    rd.FrontCounterClockwise = false;
    rd.DepthClipEnable = true;
    HR(device->CreateRasterizerState(&rd, &NoCullRS));

    // AlphaToCoverageBS
    ZeroMemory(&bd, sizeof(D3D11_BLEND_DESC));
    bd.AlphaToCoverageEnable = true;
    bd.IndependentBlendEnable = false;
    bd.RenderTarget[0].BlendEnable = false;
    bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    HR(device->CreateBlendState(&bd, &AlphaToCoverageBS));

    // TransparentBS
    ZeroMemory(&bd, sizeof(D3D11_BLEND_DESC));
    bd.AlphaToCoverageEnable          = false;
    bd.IndependentBlendEnable         = false;
    bd.RenderTarget[0].BlendEnable    = true;
    bd.RenderTarget[0].SrcBlend       = D3D11_BLEND_SRC_ALPHA;
    bd.RenderTarget[0].DestBlend      = D3D11_BLEND_INV_SRC_ALPHA;
    bd.RenderTarget[0].BlendOp        = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_ONE;
    bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    bd.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    HR(device->CreateBlendState(&bd, &TransparentBS));
}

void RenderStates::DestroyAll()
{
    ReleaseCOM(WireFrameRS);
    ReleaseCOM(NoCullRS);
    ReleaseCOM(AlphaToCoverageBS);
    ReleaseCOM(TransparentBS);
}
