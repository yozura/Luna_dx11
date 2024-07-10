#ifndef VERTEX_H
#define VERTEX_H

#include "D3DUtil.h"

namespace Vertex
{
    struct PosNormal
    {
        DirectX::XMFLOAT3 Pos;
        DirectX::XMFLOAT3 Normal;
    };
}

class InputLayoutDesc
{
public:
    static const D3D11_INPUT_ELEMENT_DESC PosNormal[2];
};

class InputLayouts
{
public:
    static void InitAll(ID3D11Device* device);
    static void DestroyAll();

    static ID3D11InputLayout* PosNormal;
};

#endif /* VERTEX_H */
