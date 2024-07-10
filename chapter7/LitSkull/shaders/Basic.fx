#include "LightHelper.fx"

cbuffer cbPerFrame
{
    DirectionalLight gDirLights[3];
    float3 gEyePosW;
    
    float gFogStart;
    float gFogRange;
    float gFogColor;
};

cbuffer cbPerObject
{
    float4x4 gWorld;
    float4x4 gWorldViewProj;
    float4x4 gWorldInvTranspose;
    float4x4 gTexTransform;
    Material gMaterial;
};

Texture2D gDiffuseMap;

SamplerState samAnisotropic
{
    Filter = ANISOTROPIC;
    MaxAnisotropy = 4;

    AddressU = WRAP;
    AddressV = WRAP;
};

struct VertexIn
{
    float3 PosL    : POSITION;
    float3 NormalL : NORMAL;
};

struct VertexOut
{
    float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    
    // Transform to world space.
    float4 newPosW = float4(vin.PosL, 1.0f);
    
    vout.PosW    = mul(newPosW, gWorld).xyz;
    vout.NormalW = mul(vin.NormalL, (float3x3)gWorldInvTranspose);
    vout.PosH    = mul(newPosW, gWorldViewProj);
    
    return vout;
}

float4 PS(VertexOut pin, uniform int gLightCount) : SV_Target
{
    pin.NormalW = normalize(pin.NormalW);
    
    float3 toEye = gEyePosW - pin.PosW;
    
    float distToEye = length(toEye);

    toEye /= distToEye;
    
    float4 ambient  = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse  = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    [unroll]
    for (int i = 0; i < gLightCount; ++i)
    {
        float4 A, D, S;
        ComputeDirectionalLight(gMaterial, gDirLights[i], pin.NormalW, toEye, A, D, S);
        
        ambient  += A;
        diffuse  += D;
        specular += S;
    }
    
    float4 litColor = ambient + diffuse + specular;
    
    litColor.a = gMaterial.Diffuse.a;

    return litColor;
}

technique11 Light1
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(1)));
    }
}

technique11 Light2
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(2)));
    }
}

technique11 Light3
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(3)));
    }
}