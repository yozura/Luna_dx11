#ifndef LIGHT_HELPER_H
#define LIGHT_HELPER_H

#include <Windows.h>
#include <DirectXMath.h>

struct DirectionalLight
{
    DirectionalLight() { ZeroMemory(this, sizeof(this)); }

    DirectX::XMFLOAT4 Ambient;
    DirectX::XMFLOAT4 Diffuse;
    DirectX::XMFLOAT4 Specular;
    DirectX::XMFLOAT3 Direction;

    float pad; 
};

struct PointLight
{
    PointLight() { ZeroMemory(this, sizeof(this)); }

    DirectX::XMFLOAT4 Ambient;
    DirectX::XMFLOAT4 Diffuse;
    DirectX::XMFLOAT4 Specular;

    DirectX::XMFLOAT3 Position;
    float Range;

    DirectX::XMFLOAT3 Att;
    float Pad;
};

struct SpotLight
{
    SpotLight() { ZeroMemory(this, sizeof(this)); }

    DirectX::XMFLOAT4 Ambient;
    DirectX::XMFLOAT4 Diffuse;
    DirectX::XMFLOAT4 Specular;

    DirectX::XMFLOAT3 Position;
    float Range;

    DirectX::XMFLOAT3 Direction;
    float Spot;

    DirectX::XMFLOAT3 Att;
    float pad;
};

struct Material
{
    Material() { ZeroMemory(this, sizeof(this)); }

    DirectX::XMFLOAT4 Ambient;
    DirectX::XMFLOAT4 Diffuse;
    DirectX::XMFLOAT4 Specular;
    DirectX::XMFLOAT4 Reflect;
};

#endif /* LIGHT_HELPER_H */