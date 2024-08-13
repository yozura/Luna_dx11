cbuffer cbPerFrame
{
    float3 gEyePosW;
    
    float gHeightScale;
    float gMaxTessDistance;
    float gMinTessDistance;
    float gMinTessFactor;
    float gMaxTessFactor;
};

cbuffer cbPerObject
{
    float4x4 gWorld;
    float4x4 gWorldInvTranspose;
    float4x4 gViewProj;
    float4x4 gWorldViewProj;
    float4x4 gTexTransform;
};

Texture2D gDiffuseMap;
Texture2D gNormalMap;

SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;

    AddressU = WRAP;
    AddressV = WRAP;
};

struct VertexIn
{
    float3 PosL    : POSITION;
    float3 NormalL : NORMAL;
    float2 Tex     : TEXCOORD;
};

struct VertexOut
{
    float4 PosH : SV_Position;
    float2 Tex  : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    
    vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
    vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;
    
    return vout;
}

struct TessVertexOut
{
    float3 PosW       : POSITION;
    float3 NormalW    : NORMAL;
    float2 Tex        : TEXCOORD;
    float  TessFactor : TESS;
};

TessVertexOut TessVS(VertexIn vin)
{
    TessVertexOut vout;
    
    vout.PosW    = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
    vout.NormalW = mul(vin.NormalL, (float3x3) gWorldInvTranspose);
    vout.Tex     = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;
    
    float d = distance(vout.PosW, gEyePosW);

    float tess = saturate((gMinTessDistance - d) / (gMinTessDistance - gMaxTessDistance));
    
    vout.TessFactor = gMinTessFactor + tess * (gMaxTessFactor - gMinTessFactor);
    
    return vout;
}

struct PatchTess
{
    float EdgeTess[3] : SV_TessFactor;
    float InsideTess  : SV_InsideTessFactor;
};

PatchTess PatchHS(InputPatch<TessVertexOut, 3> patch,
                  uint patchID : SV_PrimitiveID)
{
    PatchTess pt;
    
    pt.EdgeTess[0] = 0.5f * (patch[1].TessFactor + patch[2].TessFactor);
    pt.EdgeTess[1] = 0.5f * (patch[2].TessFactor + patch[0].TessFactor);
    pt.EdgeTess[2] = 0.5f * (patch[0].TessFactor + patch[1].TessFactor);
    pt.InsideTess = pt.EdgeTess[0];
    
    return pt;
}

struct HullOut
{
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
    float2 Tex     : TEXCOORD;
};

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchHS")]
HullOut HS(InputPatch<TessVertexOut, 3> p,
           uint i : SV_OutputControlPointID,
           uint patchID : SV_PrimitiveID)
{
    HullOut hout;
    hout.PosW = p[i].PosW;
    hout.NormalW = p[i].NormalW;
    hout.Tex = p[i].Tex;
    return hout;
}

struct DomainOut
{
    float4 PosH : SV_Position;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 Tex : TEXCOORD;
};

[domain("tri")]
DomainOut DS(PatchTess patchTess,
             float3 bary : SV_DomainLocation,
             const OutputPatch<HullOut, 3> tri)
{
    DomainOut dout;
    
    dout.PosW    = bary.x * tri[0].PosW + bary.y * tri[1].PosW + bary.z * tri[2].PosW;
    dout.NormalW = bary.x * tri[0].NormalW + bary.y * tri[1].NormalW + bary.z * tri[2].NormalW;
    dout.Tex     = bary.x * tri[0].Tex + bary.y * tri[1].Tex + bary.z * tri[2].Tex;
    
    dout.NormalW = normalize(dout.NormalW);
    
    const float MipInterval = 20.0f;
    float mipLevel = clamp((distance(dout.PosW, gEyePosW) - MipInterval) / MipInterval, 0.0f, 6.0f);
    float h = gNormalMap.SampleLevel(samLinear, dout.Tex, mipLevel).a;
    
    dout.PosW += (gHeightScale * (h - 1.0)) * dout.NormalW;
    
    dout.PosH = mul(float4(dout.PosW, 1.0f), gViewProj);
    
    return dout;
}

void PS(VertexOut pin)
{
    float4 diffuse = gDiffuseMap.Sample(samLinear, pin.Tex);
    clip(diffuse.a - 0.15f);
}

void TessPS(DomainOut pin)
{
    float4 diffuse = gDiffuseMap.Sample(samLinear, pin.Tex);
    clip(diffuse.a - 0.15f);
}

RasterizerState Depth
{
    DepthBias = 100000;
    DepthBiasClamp = 0.0f;
    SlopeScaledDepthBias = 1.0f;
};


technique11 BuildShadowMapTech
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(NULL);

        SetRasterizerState(Depth);
    }
}

technique11 BuildShadowMapAlphaClipTech
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}

technique11 TessBuildShadowMapTech
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, TessVS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetGeometryShader(NULL);
        SetPixelShader(NULL);

        SetRasterizerState(Depth);
    }
}

technique11 TessBuildShadowMapAlphaClipTech
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, TessVS()));
        SetHullShader(CompileShader(hs_5_0, HS()));
        SetDomainShader(CompileShader(ds_5_0, DS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, TessPS()));
    }
}