#ifndef VERTEX_H
#define VERTEX_H

#include "D3DUtil.h"

namespace Vertex
{
    // 32����Ʈ ���� ����ü
    struct Basic32
    {
        Basic32() : Pos(0.0f, 0.0f, 0.0f), Normal(0.0f, 0.0f, 0.0f), Tex(0.0f, 0.0f) {}
        Basic32(const DirectX::XMFLOAT3& p, const DirectX::XMFLOAT3& n, const DirectX::XMFLOAT2& t)
            : Pos(p), Normal(n), Tex(t) {}
        Basic32(float px, float py, float pz, float nx, float ny, float nz, float u, float v)
            : Pos(px, py, pz), Normal(nx, ny, nz), Tex(u, v) {}

        DirectX::XMFLOAT3 Pos;
        DirectX::XMFLOAT3 Normal;
        DirectX::XMFLOAT2 Tex;
    };

    struct TreePointSprite
    {
        DirectX::XMFLOAT3 Pos;
        DirectX::XMFLOAT2 Size;
    };
}

class InputLayoutDesc
{
public:
    static const D3D11_INPUT_ELEMENT_DESC Basic32[3];
    static const D3D11_INPUT_ELEMENT_DESC TreePointSprite[3];
};

class InputLayouts
{
public:
    static void InitAll(ID3D11Device* device);
    static void DestroyAll();

    static ID3D11InputLayout* Basic32;
    static ID3D11InputLayout* TreePointSprite;
};

#endif /* VERTEX_H */
