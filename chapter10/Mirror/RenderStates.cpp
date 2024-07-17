#include "RenderStates.h"

ID3D11RasterizerState* RenderStates::WireFrameRS     = 0;
ID3D11RasterizerState* RenderStates::NoCullRS        = 0;
ID3D11RasterizerState* RenderStates::CullClockwiseRS = 0;

ID3D11BlendState* RenderStates::AlphaToCoverageBS     = 0;
ID3D11BlendState* RenderStates::TransparentBS         = 0;
ID3D11BlendState* RenderStates::DiscardRedGreenBS     = 0;
ID3D11BlendState* RenderStates::NoRenderTargetWriteBS = 0;

ID3D11DepthStencilState* RenderStates::MarkMirrorDSS    = 0;
ID3D11DepthStencilState* RenderStates::DrawReflectDSS   = 0;
ID3D11DepthStencilState* RenderStates::NoDoubleBlendDSS = 0;

void RenderStates::InitAll(ID3D11Device* device)
{
    D3D11_RASTERIZER_DESC    rd;
    D3D11_BLEND_DESC         bd;
    D3D11_DEPTH_STENCIL_DESC dsd;

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

    //CullClockWiseRS
    ZeroMemory(&rd, sizeof(D3D11_RASTERIZER_DESC));
    rd.FillMode = D3D11_FILL_SOLID;
    rd.CullMode = D3D11_CULL_BACK;
    rd.FrontCounterClockwise = true;
    rd.DepthClipEnable = true;
    HR(device->CreateRasterizerState(&rd, &CullClockwiseRS));

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

    // DiscardRedGreenBS
    ZeroMemory(&bd, sizeof(D3D11_BLEND_DESC));
    bd.AlphaToCoverageEnable          = false;
    bd.IndependentBlendEnable         = false;
    bd.RenderTarget[0].SrcBlend       = D3D11_BLEND_SRC_COLOR;
    bd.RenderTarget[0].DestBlend      = D3D11_BLEND_INV_SRC_COLOR;
    bd.RenderTarget[0].BlendOp        = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_ONE;
    bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    bd.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_BLUE | D3D11_COLOR_WRITE_ENABLE_ALPHA;
    HR(device->CreateBlendState(&bd, &DiscardRedGreenBS));

    // NoRenderTargetWriteBS
    ZeroMemory(&bd, sizeof(D3D11_BLEND_DESC));
    bd.AlphaToCoverageEnable          = false;
    bd.IndependentBlendEnable         = false;
    bd.RenderTarget[0].BlendEnable    = false;
    bd.RenderTarget[0].SrcBlend       = D3D11_BLEND_ONE;
    bd.RenderTarget[0].DestBlend      = D3D11_BLEND_ZERO;
    bd.RenderTarget[0].BlendOp        = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_ONE;
    bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    bd.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].RenderTargetWriteMask = 0;
    HR(device->CreateBlendState(&bd, &NoRenderTargetWriteBS));

    // MarkMirrorDSS
    ZeroMemory(&dsd, sizeof(D3D11_DEPTH_STENCIL_DESC));
    dsd.DepthEnable                  = true;
    dsd.DepthWriteMask               = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsd.DepthFunc                    = D3D11_COMPARISON_LESS;
    dsd.StencilEnable                = true;
    dsd.StencilReadMask              = 0xff;
    dsd.StencilWriteMask             = 0xff;
    dsd.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
    dsd.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    dsd.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_REPLACE;
    dsd.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
    dsd.BackFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;
    dsd.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_KEEP;
    dsd.BackFace.StencilPassOp       = D3D11_STENCIL_OP_REPLACE;
    dsd.BackFace.StencilFunc         = D3D11_COMPARISON_ALWAYS;
    HR(device->CreateDepthStencilState(&dsd, &MarkMirrorDSS));

    // DrawReflectDDS
    ZeroMemory(&dsd, sizeof(D3D11_DEPTH_STENCIL_DESC));
    dsd.DepthEnable                  = true;
    dsd.DepthWriteMask               = D3D11_DEPTH_WRITE_MASK_ALL;
    dsd.DepthFunc                    = D3D11_COMPARISON_LESS;
    dsd.StencilEnable                = true;
    dsd.StencilReadMask              = 0xff;
    dsd.StencilWriteMask             = 0xff;
    dsd.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
    dsd.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    dsd.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
    dsd.FrontFace.StencilFunc        = D3D11_COMPARISON_EQUAL;
    dsd.BackFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;
    dsd.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_KEEP;
    dsd.BackFace.StencilPassOp       = D3D11_STENCIL_OP_KEEP;
    dsd.BackFace.StencilFunc         = D3D11_COMPARISON_EQUAL;
    HR(device->CreateDepthStencilState(&dsd, &DrawReflectDSS));

    // NoDoubleBlendDSS
    ZeroMemory(&dsd, sizeof(D3D11_DEPTH_STENCIL_DESC));
    dsd.DepthEnable                  = true;
    dsd.DepthWriteMask               = D3D11_DEPTH_WRITE_MASK_ALL;
    dsd.DepthFunc                    = D3D11_COMPARISON_LESS;
    dsd.StencilEnable                = true;
    dsd.StencilReadMask              = 0xff;
    dsd.StencilWriteMask             = 0xff;
    dsd.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
    dsd.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    dsd.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_INCR;
    dsd.FrontFace.StencilFunc        = D3D11_COMPARISON_EQUAL;
    dsd.BackFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;
    dsd.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_KEEP;
    dsd.BackFace.StencilPassOp       = D3D11_STENCIL_OP_INCR;
    dsd.BackFace.StencilFunc         = D3D11_COMPARISON_EQUAL;
    HR(device->CreateDepthStencilState(&dsd, &NoDoubleBlendDSS));
}

void RenderStates::DestroyAll()
{
    ReleaseCOM(WireFrameRS);
    ReleaseCOM(NoCullRS);
    ReleaseCOM(CullClockwiseRS);

    ReleaseCOM(AlphaToCoverageBS);
    ReleaseCOM(TransparentBS);
    ReleaseCOM(DiscardRedGreenBS);
    ReleaseCOM(NoRenderTargetWriteBS);

    ReleaseCOM(MarkMirrorDSS);
    ReleaseCOM(DrawReflectDSS);
    ReleaseCOM(NoDoubleBlendDSS);
}
