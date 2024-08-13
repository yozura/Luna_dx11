#ifndef VERTEX_H
#define VERTEX_H

#include "D3DUtil.h"

namespace Vertex
{
    struct Basic32
    {
        DirectX::XMFLOAT3 Pos;
        DirectX::XMFLOAT3 Normal;
        DirectX::XMFLOAT2 Tex;
    };
    
    struct PosNormalTexTan
    {
        DirectX::XMFLOAT3 Pos;
        DirectX::XMFLOAT3 Normal;
        DirectX::XMFLOAT2 Tex;
        DirectX::XMFLOAT3 TangentU;
    };
}

class InputLayoutDesc
{
public:
    static const D3D11_INPUT_ELEMENT_DESC Pos[1];
    static const D3D11_INPUT_ELEMENT_DESC Basic32[3];
    static const D3D11_INPUT_ELEMENT_DESC PosNormalTexTan[4];
};

class InputLayouts
{
public:
    static void InitAll(ID3D11Device* device);
    static void DestroyAll();

    static ID3D11InputLayout* Pos;
    static ID3D11InputLayout* Basic32;
    static ID3D11InputLayout* PosNormalTexTan;
};

#endif /* VERTEX_H */
