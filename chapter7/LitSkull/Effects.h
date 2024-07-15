#ifndef EFFECTS_H
#define EFFECTS_H

#include "D3DUtil.h"
#include "Lighthelper.h"

#pragma region Effect
class Effect
{
public:
    Effect(ID3D11Device* device, const std::wstring& filename);
    virtual ~Effect();

private:
    Effect(const Effect& rhs);
    Effect& operator=(const Effect& rhs);

protected:
    ID3DX11Effect* mFX;
};
#pragma endregion

#pragma region BasicEffect
class BasicEffect : public Effect
{
public:
    BasicEffect(ID3D11Device* device, const std::wstring& filename);
    ~BasicEffect();

    void SetWorldViewProj(DirectX::CXMMATRIX M)       { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
    void SetWorld(DirectX::CXMMATRIX M)               { World->SetMatrix(reinterpret_cast<const float*>(&M)); }
    void SetWorldInvTranspose(DirectX::CXMMATRIX M)   { WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }
    void SetTexTransform(DirectX::CXMMATRIX M)        { TexTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
    void SetEyePosW(const DirectX::XMFLOAT3& v)       { EyePosW->SetRawValue(&v, 0, sizeof(DirectX::XMFLOAT3)); }
    void SetDirLights(const DirectionalLight* lights) { DirLights->SetRawValue(lights, 0, sizeof(DirectionalLight)); }
    void SetMaterial(const Material& mat)             { Mat->SetRawValue(&mat, 0, sizeof(Material)); }
    void SetDiffuseMap(ID3D11ShaderResourceView* tex) { DiffuseMap->SetResource(tex); }

    ID3DX11EffectTechnique* Light1Tech;
    ID3DX11EffectTechnique* Light2Tech;
    ID3DX11EffectTechnique* Light3Tech;

    ID3DX11EffectMatrixVariable* WorldViewProj;
    ID3DX11EffectMatrixVariable* World;
    ID3DX11EffectMatrixVariable* WorldInvTranspose;
    ID3DX11EffectMatrixVariable* TexTransform;
    ID3DX11EffectMatrixVariable* EyePosW;
    ID3DX11EffectVariable* DirLights;
    ID3DX11EffectVariable* Mat;

    ID3DX11EffectShaderResourceVariable* DiffuseMap;
};
#pragma endregion

#pragma region Effects
class Effects
{
public:
    static void InitAll(ID3D11Device* device);
    static void DestroyAll();

    static BasicEffect* BasicFX;
};
#pragma endregion

#endif /* EFFECTS_H */
