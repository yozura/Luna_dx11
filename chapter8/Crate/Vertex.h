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
    
    // 32바이트 정점 구조체
    struct Basic32
    {
        DirectX::XMFLOAT3 Pos;
        DirectX::XMFLOAT3 Normal;
        DirectX::XMFLOAT2 Tex;
    };
}

class InputLayoutDesc
{
public:
    static const D3D11_INPUT_ELEMENT_DESC PosNormal[2];
    static const D3D11_INPUT_ELEMENT_DESC Basic32[3];
};

class InputLayouts
{
public:
    static void InitAll(ID3D11Device* device);
    static void DestroyAll();

    static ID3D11InputLayout* PosNormal;
    static ID3D11InputLayout* Basic32;
};

#endif /* VERTEX_H */
