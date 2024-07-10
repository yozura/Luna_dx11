#include "Effects.h"

#pragma region Effect
Effect::Effect(ID3D11Device* device, const std::wstring& filename)
    : mFX(0)
{
    std::ifstream fin(filename, std::ios::binary);
    
    fin.seekg(0, std::ios::end);
    int size = (int)fin.tellg();
    fin.seekg(0, std::ios::beg);
    std::vector<char> compiledShader(size);

    fin.read(&compiledShader[0], size);
    fin.close();

    HR(D3DX11CreateEffectFromMemory(&compiledShader[0], size, 0, device, &mFX));
}

Effect::~Effect()
{
    ReleaseCOM(mFX);
}
#pragma endregion

#pragma region BasicEffect
BasicEffect::BasicEffect(ID3D11Device* device, const std::wstring& filename)
    : Effect(device, filename)
{
    Light1Tech        = mFX->GetTechniqueByName("Light1");
    Light2Tech        = mFX->GetTechniqueByName("Light2");
    Light3Tech        = mFX->GetTechniqueByName("Light3");
    WorldViewProj     = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
    World             = mFX->GetVariableByName("gWorld")->AsMatrix();
    WorldInvTranspose = mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
    EyePosW           = mFX->GetVariableByName("gEyePosW")->AsMatrix();
    DirLights         = mFX->GetVariableByName("gDirLights");
    Mat               = mFX->GetVariableByName("gMaterial");
}

BasicEffect::~BasicEffect()
{
}
#pragma endregion

#pragma region Effects
BasicEffect* Effects::BasicFX = 0;

void Effects::InitAll(ID3D11Device* device)
{
    BasicFX = new BasicEffect(device, L"shaders/Basic.cso");
}

void Effects::DestroyAll()
{
    SafeDelete(BasicFX);
}
#pragma endregion