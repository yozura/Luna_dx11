#include "LightHelper.fx"

cbuffer cbPerFrame
{
    DirectionalLight gDirLights[3];
    float3 gEyePosW;

    float  gFogStart;
    float  gFogRange;
    float4 gFogColor;
};

cbuffer cbPerObject
{
    float4x4 gWorld;
    float4x4 gWorldInvTranspose;
    float4x4 gWorldViewProj;
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
    float3 PosL : POSITION;
};

struct VertexOut
{
    float3 PosL : POSITION;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    vout.PosL = vin.PosL;
    return vout;
}

struct PatchTess
{
    float EdgeTess[4]   : SV_TessFactor;
    float InsideTess[2] : SV_InsideTessFactor;
};

PatchTess ConstantHS(InputPatch<VertexOut, 16> patch, uint PatchID : SV_PrimitiveID)
{
    PatchTess pt;
    
    pt.EdgeTess[0] = 25;
    pt.EdgeTess[1] = 25;
    pt.EdgeTess[2] = 25;
    pt.EdgeTess[3] = 25;
    
    pt.InsideTess[0] = 25;
    pt.InsideTess[1] = 25;
    
    return pt;
}

struct HullOut
{
    float3 PosL : POSITION;
};

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(16)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
HullOut HS(InputPatch<VertexOut, 16> p, 
           uint i : SV_OutputControlPointID,
           uint patchID : SV_PrimitiveID)
{
    HullOut hout;
    hout.PosL = p[i].PosL;
    return hout;
}

struct DomainOut
{
    float4 PosH : SV_POSITION;
};

float4 BernsteinBasis(float t)
{
    float invT = 1.0f - t;
    
    return float4(invT * invT * invT,
                  3.0f * t * invT * invT,
                  3.0f * t * t * invT,
                  t * t * t);
}

float3 CubicBezierSum(const OutputPatch<HullOut, 16> bezPatch, float4 basisU, float4 basisV)
{
    float3 sum = float3(0.0f, 0.0f, 0.0f);
    sum  = basisV.x * (basisU.x * bezPatch[0].PosL + basisU.y * bezPatch[1].PosL + basisU.z * bezPatch[2].PosL + basisU.w * bezPatch[3].PosL);
    sum += basisV.y * (basisU.x * bezPatch[4].PosL + basisU.y * bezPatch[5].PosL + basisU.z * bezPatch[6].PosL + basisU.w * bezPatch[7].PosL);
    sum += basisV.z * (basisU.x * bezPatch[8].PosL + basisU.y * bezPatch[9].PosL + basisU.z * bezPatch[10].PosL + basisU.w * bezPatch[11].PosL);
    sum += basisV.w * (basisU.x * bezPatch[12].PosL + basisU.y * bezPatch[13].PosL + basisU.z * bezPatch[14].PosL + basisU.w * bezPatch[15].PosL);
    return sum;
}

float4 dBernsteinBasis(float t)
{
    float invT = 1.0f - t;
    
    return float4(-3 * invT * invT,
                  3 * invT * invT - 6 * invT,
                  6 * t * invT - 3 * t * t,
                  3 * t * t);
}

[domain("quad")]
DomainOut DS(PatchTess patchTess,
             float2 uv : SV_DomainLocation,
             const OutputPatch<HullOut, 16> bezPatch)
{
    DomainOut dout;
    
    float4 basisU = BernsteinBasis(uv.x);
    float4 basisV = BernsteinBasis(uv.y);
    
    float3 p = CubicBezierSum(bezPatch, basisU, basisV);
    
    dout.PosH = mul(float4(p, 1.0f), gWorldViewProj);
    
    return dout;
}

float4 PS(DomainOut pin) : SV_Target
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

technique11 Tess
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}